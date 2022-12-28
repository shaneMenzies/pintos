#ifndef THREADING_H
#define THREADING_H

#include "interrupts/interrupts.h"
#include "libk/asm.h"
#include "libk/callable.h"
#include "libk/functional.h"
#include "libk/ostream.h"
#include "libk/vector.h"
#include "memory/addressing.h"
#include "memory/chunking_predef.h"
#include "memory/common_region.h"
#include "memory/p_memory.h"
#include "terminal/terminal.h"
#include "threading/topology.h"
#include "time/timer.h"

#include <cpuid.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

struct allocation_manager;

namespace acpi {
struct srat_table;
struct madt_table;
} // namespace acpi

namespace threading {
struct thread_scheduler;
struct process;

struct new_thread_startup_info {
    void*  thread_start;
    void** thread_target;
    void** thread_stack_top;
};
extern struct new_thread_startup_info thread_startup_info;

void thread_init();

void enter_sleep();

void start_threads();

void end_of_task();

enum thread_load_types : uint32_t {
    integer    = 0,
    floating   = 1,
    memory     = 2,
    peripheral = 3,
    pci_bus    = 4,
    io_bus     = 5,
    system     = 6,
    generic    = 7,
};

enum process_waiting_type : int32_t {
    skip_task  = -1,
    none       = 0,
    lazy_check = 1,
};

struct process {
    processor_state saved_state;

    unsigned int id = 0;
    uint32_t     load_type;

    process_waiting_type waiting;

    unsigned int priority;
    unsigned int rounds;

    unsigned int priority_count;

    std_k::callable<void>* main;
    std_k::ostream         out_stream;

    allocation_manager*         task_allocation = 0;
    paging::address_space*      task_space;
    size_t                      task_space_index;
    paging::page_level_4_table* lvl4_table;

    thread_scheduler*       scheduler;
    process*                parent_task;
    std_k::vector<process*> children;

    process(uint8_t load_type, unsigned int target_priority,
            unsigned int rounds, size_t stack_size,
            std_k::callable<void>* target, process* parent = 0)
        : load_type(load_type)
        , priority(target_priority)
        , rounds(rounds)
        , main(target)
        , out_stream(active_terminal)
        , parent_task(parent) {

        // Set appropriate address space for this task
        if (parent_task == nullptr) {
            task_space = new paging::address_space;
        } else {
            // Use parent task's address space
            task_space = parent_task->task_space;

            // Set this into the parent task's child tasks
            parent_task = common_region::current_process;
            parent_task->children.push_back(this);
        }

        // Need to add new table to address space
        task_space_index = task_space->get_new_table();
        lvl4_table       = task_space->shared_tables[task_space_index];

        // Create and set this task's own stack
        stack_size += 16; // Stack will need space for the scheduler's return
                          // address, an ending function, and red zone below
        saved_state.rsp
            = (uint64_t)((uintptr_t)malloc(stack_size + 128) + stack_size);

        // Map this process info into the process' address space
        task_space->map_region_to((uintptr_t)this,
                                  (uintptr_t)common_region::current_process,
                                  sizeof(process), task_space_index);

        // Set initial target
        prepare_wrapper();
    }

    static void init_wrapper(process* target) {
        target->main->call();
    }

    void prepare_wrapper() {
        saved_state.rip = (uint64_t)init_wrapper;
        saved_state.rdi = (uint64_t)this;
    }

    void* operator new(size_t size) {
        // Need to allocate on 16-byte boundary
        return aligned_alloc(size, 16);
    }

    bool check_waiting() {
        // Need to check value of waiting
        switch (waiting) {
            case none:
                // Not waiting for anything
                return true;

            case skip_task:
                // Skip this task
                return false;

            case lazy_check:
                // Refer to lazy check method
                return lazy_timing_check();

            default:
                // Skip task
                return false;
        }
    }

    bool lazy_timing_check();
};

struct thread_scheduler {
    logical_core* owner;
    unsigned int  total_load;
    union {
        uint8_t  load_field[8];
        uint64_t type_load = 0;
    };

    std_k::vector<process*> tasks;

    unsigned int current_task_index = 0;

#define THREAD_TIMER_DEFAULT_RATE 10000 // 100 microsecond interval / 10 kHz
#define SCHEDULING_DEFAULT_RATE   100   // 10 millisecond interval / 100 Hz
    timer<>* local_timer;

    std_k::preset_function<void(thread_scheduler*, general_regs_state*,
                                interrupt_frame*)>
        scheduling_function;

    volatile bool in_sleep = false;
    void          sleep() {
        in_sleep = true;
        enable_interrupts();

        while (1) { asm volatile("hlt"); }
    }

    thread_scheduler() {}
    thread_scheduler(logical_core* owner, unsigned int initial_load)
        : owner(owner)
        , total_load(initial_load)
        , local_timer(&owner->local_apic)
        , scheduling_function(run, this, 0, 0) {
        local_timer->push_task_rate((unsigned long)SCHEDULING_DEFAULT_RATE,
                                    &scheduling_function, -1);
    }

    process* current_task() {
        if (tasks.empty()) {
            return nullptr;
        } else {
            return (tasks[current_task_index]);
        }
    }

    void add_task(process* new_task) {

        // Set task's scheduler
        new_task->scheduler = this;

        // Adjust load
        total_load += new_task->priority;
        for (int i = 0; i < 8; i++) {
            if (new_task->load_type & (1 << i)) { load_field[i]++; }
        }

        // Push the scheduler's address onto the task's stack
        new_task->saved_state.rsp -= sizeof(thread_scheduler*);
        *((uint64_t*)new_task->saved_state.rsp) = (uint64_t)this;
        new_task->saved_state.rsp -= sizeof(void (*)());
        *((uint64_t*)new_task->saved_state.rsp) = (uint64_t)end_of_task;

        // Add it to the cycle
        tasks.push_back(new_task);
    }

    void remove_task(unsigned int index) {
        process* removed_task = tasks[index];
        tasks.erase(index);

        // Adjust load
        total_load -= removed_task->priority;
        for (int i = 0; i < 8; i++) {
            if (removed_task->load_type & (1 << i)) { load_field[i]--; }
        }

        delete removed_task;
    }

    static void run(thread_scheduler* target, general_regs_state* task_regs,
                    interrupt_frame* frame) {

        // Check to see if the core is in sleep with a task available
        if (target->in_sleep) {
            if (!target->tasks.empty()) {
                // Leave sleep
                target->in_sleep = false;

                // Start work on the task
                target->current_task_index = 0;
                target->tasks[0]->saved_state.load_state(task_regs, frame);
            }
        } else {

            threading::process* active_task = target->current_task();
            if (active_task == nullptr) { return; }

            size_t previous_index = target->current_task_index;
            if (!(active_task->priority_count < active_task->priority)) {
                if (!target->tasks.empty()) {
                    // Move to next task
                    active_task->saved_state.save_state(task_regs, frame);

                    while (1) {
                        target->current_task_index++;
                        if (target->current_task_index > target->tasks.end()) {
                            target->current_task_index = 0;
                        }

                        if (target->current_task_index == previous_index) {
                            // No task ready to replace
                            frame->return_instruction = (uint64_t)enter_sleep;
                            return;
                        }

                        if (target->current_task()->waiting == skip_task) {
                            continue;
                        } else if (target->current_task()->waiting == lazy_check
                                   && !target->current_task()
                                           ->lazy_timing_check()) {
                            continue;
                        } else {
                            break;
                        }
                    }

                    target->current_task()->priority_count = 0;
                    target->current_task()->saved_state.load_state(task_regs,
                                                                   frame);
                    return;
                } else {
                    // No other tasks, so repeat this one
                    active_task->priority_count = 0;
                }
            } else {
                // Staying with this task
                active_task->priority_count++;
            }
        }
    }

    void yield_current(general_regs_state* task_regs, interrupt_frame* frame) {

        threading::process* active_task = current_task();
        if (active_task == nullptr) { return; }

        size_t previous_index = current_task_index;
        active_task->saved_state.save_state(task_regs, frame);

        if (tasks.size() > 1) {
            // Move to next task
            active_task->saved_state.save_state(task_regs, frame);

            while (1) {
                current_task_index++;
                if (current_task_index > tasks.end()) {
                    current_task_index = 0;
                }

                if (current_task_index == previous_index) {
                    // No task ready to replace
                    frame->return_instruction = (uint64_t)enter_sleep;
                    return;
                }

                if (current_task()->waiting == skip_task) {
                    continue;
                } else if (current_task()->waiting == lazy_check
                           && !current_task()->lazy_timing_check()) {
                    continue;
                } else {
                    break;
                }
            }

            current_task()->priority_count = 0;
            current_task()->saved_state.load_state(task_regs, frame);
        } else {
            // No other tasks, so enter sleep
            frame->return_instruction = (uint64_t)enter_sleep;
        }
    }
};

struct system_scheduler {
    unsigned int next_id = 0;

    unsigned int num_threads;
    bool         shared_threads;

    thread_scheduler* threads;

    void send_task(process* target) {

        // Give the task a new id
        target->id = next_id;
        next_id++;

        // Find the thread to send the task to
        thread_scheduler* target_scheduler = threads;
        for (unsigned int i = 1; i < num_threads; i++) {
            if (threads[i].total_load < target_scheduler->total_load) {
                target_scheduler = &threads[i];
            }
        }

        // Send the task
        target_scheduler->add_task(target);
    }
};

extern struct system_scheduler main_scheduler;

} // namespace threading

#endif

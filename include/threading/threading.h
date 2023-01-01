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
#include "process_def.h"
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

            process* active_task = target->current_task();
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

        process* active_task = current_task();
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

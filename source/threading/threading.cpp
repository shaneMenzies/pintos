/**
 * @file threading.cpp
 * @author Shane Menzies
 * @brief Multithreading support
 * @date 07/27/21
 *
 *
 */

#include "threading.h"

#include "interrupts/interrupts.h"
#include "libk/asm.h"
#include "memory/chunking.h"
#include "memory/p_memory.h"
#include "process_def.h"
#include "system/acpi.h"
#include "terminal/terminal.h"
#include "time/timer.h"
#include "topology.h"

void __attribute__((noreturn)) cpu_sleep_state() {
    enable_interrupts();

    while (1) { asm volatile("hlt \n\t"); }
}

namespace threading {

struct new_thread_startup_info thread_startup_info;

process_list_t process_list;

system_scheduler_t system_scheduler;

void thread_init() {

    // Enable floating point instructions
    fpu_init();

    // Find current thread
    logical_core* thread = current_thread();
    thread->gdt.load_gdt();
    thread->ist.load_ist();

    // Load the Interrupt Descriptor Table
    set_idt(interrupts::idt_table, interrupts::IDT_SIZE);

    // Setup local apic
    new (&thread->local_apic) apic<true, false>();

    // Find this core's scheduler
    thread_scheduler* scheduler = thread->scheduler;

    if (scheduler == 0) { asm volatile("cli\n\t hlt"); }

    // Setup the scheduler
    scheduler = new (scheduler) thread_scheduler(current_thread());

    // Enter this core's sleep
    scheduler->enter_sleep();
}

void enter_sleep() {
    // Find this core's scheduler
    thread_scheduler* scheduler = current_thread()->scheduler;

    // Go to this core's sleep
    scheduler->enter_sleep();
}

void start_threads() {
    // Loop through each logical thread of the system
    for (unsigned int i = 0; i < topology.num_logical; i++) {

        // Create this threads scheduler
        topology.threads[i].scheduler = new thread_scheduler;

        // Initialize this thread's memory piles
        topology.threads[i].memory_piles
            = new chunking::chunk_pile[chunking::NUM_MEMORY_PILES];
        for (unsigned int pile = 0; pile < chunking::NUM_MEMORY_PILES; pile++) {
            new (&topology.threads[i].memory_piles[pile])
                chunking::chunk_pile(pile);
        }

        // Start the thread and send it to the thread initialization,
        // (boot thread will go by itself)
        if (!topology.threads[i].boot_thread) {
            topology.threads[i].start_thread(thread_init);
        }
    }
}

void thread_scheduler::run(thread_scheduler*   target,
                           general_regs_state* task_regs,
                           interrupt_frame*    frame) {
    process* new_task = system_scheduler.get();

    // If scheduler has no tasks, it returns null without locking
    if (new_task != nullptr) {
        // Swap back old task
        if (target->current_task != nullptr) {
            target->current_task->saved_state.save_state(task_regs, frame);
            system_scheduler.add_process(target->current_task);
        }

        // Start work on new task
        target->current_task = new_task;
        target->current_task->saved_state.load_state(task_regs, frame);

        // active_terminal->tprintf("Scheduler for cpu%x swapping to %p \n",
        //                          target->local_timer->id,
        //                          target->current_task->main);
    } else if (target->current_task == nullptr) {
        // Send cpu to sleep state if there's no task at all
        frame->return_instruction = (uint64_t)cpu_sleep_state;
    }

    // Reset scheduling timer
    int schedule_time = 1;
    if (target->current_task != nullptr)
        schedule_time = target->current_task->priority;

    unsigned long schedule_rate = SCHEDULING_DEFAULT_RATE / schedule_time;
    if (schedule_rate < 1) schedule_rate = 1;

    target->scheduling_timer_task = target->local_timer->push_task_rate(
        schedule_rate, &target->scheduling_function, 1);

    // Will return to loaded state
    return;
}

void thread_scheduler::yield_current(general_regs_state* task_regs,
                                     interrupt_frame*    frame) {
    // Need to cancel run timer early
    scheduling_timer_task->rounds = 0;

    // Check if this task is actually finished
    if (current_task->rounds == 0) {
        if (!current_task->config.wait_on_end) { delete current_task; }
        current_task = nullptr;

        // active_terminal->tprintf("Scheduler for cpu%x finished task \n",
        //                          local_timer->id);
    }

    run(this, task_regs, frame);
}

} // namespace threading

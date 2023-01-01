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
#include "libk/misc.h"
#include "libk/sorting.h"
#include "memory/addressing.h"
#include "memory/allocation_manager.h"
#include "memory/chunking.h"
#include "memory/p_memory.h"
#include "memory/paging.h"
#include "process_def.h"
#include "system/acpi.h"
#include "system/pintos_std.h"
#include "time/timer.h"
#include "topology.h"

namespace threading {

struct new_thread_startup_info thread_startup_info;

struct system_scheduler main_scheduler;

void thread_init() {

    // Enable floating point instructions
    fpu_init();

    // Load the Interrupt Descriptor Table
    set_idt(interrupts::idt_table, interrupts::IDT_SIZE);

    // Find current thread
    logical_core* thread = current_thread();

    // Setup local apic
    new (&thread->local_apic) apic<true, false>();

    // Find this core's scheduler
    thread_scheduler* scheduler = thread->scheduler;

    if (scheduler == 0) { asm volatile("cli\n\t hlt"); }

    // Setup the scheduler
    scheduler = new (scheduler) thread_scheduler(current_thread(), 0);

    // Enter this core's sleep
    scheduler->sleep();
}

void enter_sleep() {
    // Find this core's scheduler
    thread_scheduler* scheduler = current_thread()->scheduler;

    // Go to this core's sleep
    scheduler->sleep();
}

void start_threads() {
    // Set up the system's scheduler
    main_scheduler.num_threads = topology.num_logical;
    main_scheduler.shared_threads
        = (topology.num_logical > topology.num_physical);
    main_scheduler.threads = new thread_scheduler[main_scheduler.num_threads];

    // Loop through each logical thread of the system
    for (unsigned int i = 0; i < topology.num_logical; i++) {

        // Create this threads scheduler
        topology.threads[i].scheduler = &main_scheduler.threads[i];

        // Initialize this thread's memory piles
        topology.threads[i].memory_piles
            = new chunking::chunk_pile[chunking::NUM_MEMORY_PILES];
        for (unsigned int pile = 0; pile < chunking::NUM_MEMORY_PILES; pile++) {
            topology.threads[i].memory_piles[pile] = chunking::chunk_pile(pile);
        }

        // Start the thread and send it to the thread initialization,
        // (boot thread will go by itself)
        if (!topology.threads[i].boot_thread) {
            topology.threads[i].start_thread(thread_init);
        }
    }
}

void end_of_task() {
    // Task should have returned with the address of the scheduler still left on
    // the stack
    thread_scheduler* scheduler    = ((thread_scheduler**)get_rbp())[1];
    process*          current_task = scheduler->current_task();

    // Check number of times the task should be executed
    if (current_task->rounds) {
        // Not an infinite task, so decrease the completed rounds
        current_task->rounds--;

        // Is the task now finished?
        if (current_task->rounds == 0) {
            // Run completion task if it exists

            // Delete the task and start next task
            scheduler->remove_task(scheduler->current_task_index);

            if (scheduler->tasks.empty()) {
                // No more tasks left, so enter sleep
                scheduler->sleep();
            } else if (scheduler->current_task_index
                       >= scheduler->tasks.size()) {
                scheduler->current_task_index = 0;
            }
        }
    }

    // Repush the scheduler address and jump back into the task
    push_64((uint64_t)end_of_task);
    scheduler->current_task()->main->call();
}

bool process::lazy_timing_check() {
    // Get required timestamp from top of stack
    uint64_t target_time = saved_state.peek(0);

    // Get current timer timestamp
    uint64_t current_time = scheduler->local_timer->now();

    if (current_time >= target_time) {
        // Done waiting, can continue with task
        waiting = none;
        return true;
    } else {
        return false;
    }

    /**
     * Note:
     *  Technically this could cause a problem when a buffer overflow occurs,
     *  but even with a timer rate of 1 GHz, that would take over 500 years
     *
     *  If you are reading this at a point in time where a timer with a rate
     *  over 10-100 GHz, add another check of the actual elapsed time to
     *  this.
     */
}
} // namespace threading

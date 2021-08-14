/**
 * @file timer.cpp
 * @author Shane Menzies
 * @brief Organization of timed tasks to be run repeatedly
 * @date 04/06/21
 * 
 * 
 */

#include "timer.h"

#include "interrupts.h"
#include "io.h"

const unsigned long int PIT_OSC_RATE = 1193182;

namespace timer {

    hpet_info* hpet_timer = 0;
    bool hpet_periodic = false;

    timer::timer(unsigned long int target_rate, unsigned int task_limit)
        : clock_rate(target_rate), task_limit(task_limit) 
    {

        // Allocate space for tasks
        tasks = (timer_task*) malloc((sizeof(timer_task) * task_limit));
    }

    timer::~timer() {

        // Unallocate space used for tasks
        free(tasks);
    }

    timer* sys_int_timer = 0;

    void sys_timer_init(unsigned long int rate, acpi::hpet_table* hpet) {

        // Create system timer
        sys_int_timer = new timer(rate, 1024);

        if ((uintptr_t)hpet) {
            // HPET setup
            hpet_timer = new hpet_info(hpet);

            // Disable HPET for setup
            *((volatile uint64_t*)((uintptr_t)hpet_timer->address + 0x10)) &= ~(1);

            // Need to setup a comparator to start triggering interrupts,
            // preferrably one which is periodic-capable
            hpet_comparator* target_comparator = &hpet_timer->comparators[0];
            for (unsigned int i = 0; i < hpet_timer->num_comparators; i++) {

                if (hpet_timer->comparators[i].periodic_capable) {
                    target_comparator = &hpet_timer->comparators[i];
                    hpet_periodic = true;
                    break;
                }
            }

            // Route timer through appropriate interrupt line
            uint8_t target_line = 0;
            while(!(target_comparator->valid_irqs & (1<<target_line))) {target_line++;}
            target_comparator->current_irq = target_line;
            target_comparator->used = true;
            target_comparator->address[0] = ((target_comparator->address[0] & ~(0x1f<<9)) | (target_line << 9));

            // Calculate the desired period for the requested rate
            uint64_t hpet_tick_difference = hpet_timer->rate / rate;
            if (hpet_tick_difference < hpet_timer->min_tick) {
                hpet_tick_difference = hpet_timer->min_tick;
            }

            // Different setup for periodic and non-periodic modes
            if (hpet_periodic) {
                // Enable periodic mode on this comparator
                target_comparator->address[0] |= (1<<3) | (1<<6);

                // Write initial value for comparator register
                target_comparator->address[1] = hpet_tick_difference;

                // Write the desired period to comparator register
                target_comparator->address[1] = hpet_tick_difference;

                interrupts::set_interrupt(interrupts::IRQ_BASE + target_line, interrupts::gate_type::INT_GATE_64, interrupts::hpet_periodic_int, false);
            } else {
                // Write the desired period to comparator register
                target_comparator->address[1] = hpet_tick_difference;

                interrupts::set_interrupt(interrupts::IRQ_BASE + target_line, interrupts::gate_type::INT_GATE_64, interrupts::hpet_oneshot_int, false);
            }

            // Can enable the target comparator
            target_comparator->address[0] |= (1<<2);

            // Reset HPET counter to 0
            volatile uint64_t* hpet_register = (volatile uint64_t*)((uintptr_t)hpet_timer->address + 0xf0);
            *hpet_register = 0;

            // Can now enable the HPET main counter
            *((volatile uint64_t*)((uintptr_t)hpet_timer->address + 0x10)) |= 1;

        } else {
            // Legacy PIT setup

            // Determine the 16-bit divider for the PIT
            uint16_t divider;
            if (rate <= 19) {
                divider = 0;
            } else {
                divider = PIT_OSC_RATE / rate;
            }
            
            // Make the PIT command (channel 0, lo/hi byte access, mode 2, binary mode)
            unsigned char command = 0b00110110;
            out_byte(command, PIT_CMD);

            // send the divider in two parts (low first)
            out_byte((unsigned char)(divider & 0x0ff), PIT_CH0);
            out_byte((unsigned char)((divider & 0xff00) >> 8), PIT_CH0);

            // Set appropriate interrupt
            interrupts::set_interrupt(OFFSET_1 + 0, interrupts::gate_type::INT_GATE_64, interrupts::irq_0);
        }
    }

    void timer::push_task(unsigned long int rate, void (*task)()) {

        // Check if space is free for this task
        if (num_tasks < task_limit) {
            // Create the task
            int new_id = num_tasks;
            tasks[new_id].target_rate = rate;
            tasks[new_id].runner = task;
            tasks[new_id].class_task = false;
            tasks[new_id].task_id = new_id;
            tasks[new_id].divider = clock_rate / rate;
            tasks[new_id].count = tasks[new_id].divider;
            num_tasks++;

        } else {
            return;
        }
    }

    void timer::push_task(unsigned long int rate, void (*task)(timer*)) {

        // Check if space is free for this task
        if (num_tasks < task_limit) {
            // Create the task
            int new_id = num_tasks;
            tasks[new_id].target_rate = rate;
            tasks[new_id].runner = (void (*)())task;
            tasks[new_id].class_task = true;
            tasks[new_id].task_id = new_id;
            tasks[new_id].divider = clock_rate / rate;
            tasks[new_id].count = tasks[new_id].divider;
            num_tasks++;

        } else {
            return;
        }
    }

    void timer::pop_task() {

        num_tasks--;
    }

    void timer::run_tasks() {

        for (unsigned int current_task_id = 0; current_task_id < num_tasks; current_task_id++) {

            if ((--tasks[current_task_id].count) == 0) {
                if (tasks[current_task_id].class_task) {
                    ((void (*)(timer*))tasks[current_task_id].runner)(this);
                } else {
                    tasks[current_task_id].runner();
                }
                tasks[current_task_id].count = tasks[current_task_id].divider;
            }
        }
    }

    void end_sleep(timer* target) {
        target->sleep_active = false;
    }

    void timer::sleep(double seconds) {

        seconds *= static_cast<double>(clock_rate);
        unsigned long int converted_count = static_cast<unsigned long int>(seconds);

        // Create new sleep task
        // Check if space is free for this task
        if (num_tasks < task_limit) {

            // Set sleep flag
            sleep_active = true;

            // Create the task
            int new_id = num_tasks;
            tasks[new_id].target_rate = 0;
            tasks[new_id].runner = (void (*)())&end_sleep;
            tasks[new_id].class_task = true;
            tasks[new_id].task_id = new_id;
            tasks[new_id].divider = 0;
            tasks[new_id].count = (converted_count ? converted_count : 1);
            num_tasks += 1;

            // Wait until the sleep flag 
            while(1) {
                if (sleep_active) {
                    break;
                } else {
                    asm volatile ("nop");
                }
            }

            // Sleep is over, can pop the added task and return
            pop_task();
            return;

        } else {
            return;
        }
    }

}

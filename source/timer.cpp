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

const unsigned int PIT_OSC_RATE = 1193182;

namespace timer {

    timer::timer(unsigned int target_rate, unsigned int task_limit)
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

    void sys_timer_init(unsigned int rate) {

        // Create system timer
        sys_int_timer = new timer(rate, 1024);

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
    }

    void timer::push_task(unsigned int rate, void (*task)()) {

        // Check if space is free for this task
        if (num_tasks < task_limit) {
            // Create the task
            int new_id = num_tasks;
            tasks[new_id].target_rate = rate;
            tasks[new_id].runner = task;
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

            if (tasks[current_task_id].count-- == 0) {
                tasks[current_task_id].runner();
                tasks[current_task_id].count = tasks[current_task_id].divider;
            }
        }
    }

}

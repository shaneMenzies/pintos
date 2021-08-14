#ifndef TIMER_H
#define TIMER_H

#include "io.h"
#include "interrupts.h"
#include "acpi.h"

namespace timer {

extern hpet_info* hpet_timer;

class timer_task {

    friend class timer;

    private:
        unsigned long int divider;
        unsigned long int count;
        bool class_task = false;

    public:
        void (*runner)();
        unsigned int target_rate;
        int task_id;

};

class timer {

    private:
        const unsigned long int clock_rate;
        const unsigned int task_limit;
        unsigned int num_tasks = 0;
        timer_task* tasks;
        volatile bool sleep_active = false;

    public:
        timer(unsigned long int target_rate, unsigned int task_limit);
        ~timer();
        void push_task(unsigned long int rate, void (*task)());
        void push_task(unsigned long int rate, void (*task)(timer*));
        void pop_task();
        void run_tasks();
        void sleep(double seconds);

        friend void end_sleep(timer* target);
};

void sys_timer_init(unsigned long int rate, acpi::hpet_table* hpet = 0);

extern timer* sys_int_timer;

}

#endif
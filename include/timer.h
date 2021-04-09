#ifndef TIMER_H
#define TIMER_H

namespace timer {

class timer_task {

    friend class timer;

    private:
        unsigned int divider;
        unsigned int count;

    public:
        void (*runner)();
        unsigned int target_rate;
        int task_id;

};

class timer {

    private:
        const unsigned int clock_rate;
        const unsigned int task_limit;
        unsigned int num_tasks = 0;
        timer_task* tasks;

    public:
        timer(unsigned int target_rate, unsigned int task_limit);
        ~timer();
        void push_task(unsigned int rate, void (*task)());
        void pop_task();
        void run_tasks();

};



void sys_timer_init(unsigned int rate);

extern timer* sys_int_timer;

}

#endif
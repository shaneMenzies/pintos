#ifndef PINTOS_TIMER_H
#define PINTOS_TIMER_H

#include "libk/callable.h"
#include "libk/functional.h"

template<typename timestamp_type = uint64_t> class timer {
  public:
    using timestamp = timestamp_type;

  protected:
    struct task {

        std_k::callable<void>* target;
        timestamp              interval = 0;
        int                    rounds   = -1;
        timestamp              time;

        task() {}
        task(std_k::callable<void>* task, timestamp refresh_interval,
             int rounds, timestamp time)
            : target(task)
            , interval(refresh_interval)
            , rounds(rounds)
            , time(time) {}

        friend bool operator<(const task& lhs, const task& rhs) {
            return (lhs.time < rhs.time);
        }
        friend bool operator>(const task& lhs, const task& rhs) {
            return (rhs < lhs);
        }
    };

  public:
    virtual void push_task_sec(double seconds, std_k::callable<void>* task,
                               int rounds = -1)
        = 0;
    virtual void push_task_rate(unsigned long int      rate,
                                std_k::callable<void>* task, int rounds = -1)
        = 0;
    virtual void push_task_interval(timestamp              interval,
                                    std_k::callable<void>* task,
                                    int                    rounds = -1)
        = 0;

    virtual timestamp convert_sec(double seconds) const = 0;
    virtual timestamp convert_rate(uint64_t rate) const = 0;

    virtual timestamp   now() const   = 0;
    virtual void        run()         = 0;
    virtual inline bool empty() const = 0;

    void sleep(double seconds) {

        // Set sleep flag
        bool sleep_flag = true;

        // Create the task
        std_k::preset_function<void(bool*)> sleep_task(
            (void (*)(bool*))[](bool* flag) { *flag = false; }, &sleep_flag);
        push_task_sec(seconds, &sleep_task, 1);

        // Wait until the sleep flag is cleared
        while (1) {
            if (!sleep_flag) {
                break;
            } else {
                asm volatile("nop");
            }
        }
    };
};

void legacy_timer_init(unsigned long int rate);

#endif

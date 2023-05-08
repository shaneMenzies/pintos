#ifndef THREADING_H
#define THREADING_H

#include "interrupts/interrupts.h"
#include "libk/asm.h"
#include "libk/avl_tree.h"
#include "libk/callable.h"
#include "libk/functional.h"
#include "libk/mutex.h"
#include "libk/ostream.h"
#include "libk/queue.h"
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

class allocation_manager;

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

using pid_t = uint64_t;

extern class process_list_t {
  private:
    struct pid_pointer_relation {
        pid_t    pid;
        process* target;

        pid_pointer_relation(pid_t pid, process* target)
            : pid(pid)
            , target(target) {}

        bool operator<(const pid_pointer_relation& rhs) const {
            return (pid < rhs.pid);
        }
        bool operator==(const pid_pointer_relation& rhs) const {
            return (pid == rhs.pid);
        }
        bool operator>(const pid_pointer_relation& rhs) const {
            return (pid > rhs.pid);
        }
    };

    std_k::shared_mutex                   lock;
    pid_t                                 next_pid = 1;
    std_k::avl_tree<pid_pointer_relation> relations;

  public:
    process* find_process(pid_t pid) {

        lock.lock_shared();
        process* target
            = relations.find(pid_pointer_relation(pid, nullptr))->value.target;
        lock.unlock_shared();
        return target;
    }

    pid_t add_process(process* target) {

        pid_t new_pid = __atomic_fetch_add(&next_pid, 1, __ATOMIC_RELAXED);

        lock.lock();

        auto new_relation
            = new std_k::avl_node(pid_pointer_relation(new_pid, target));

        relations.insert(new_relation);

        lock.unlock();
        return new_pid;
    }

    void remove_process(pid_t pid) {
        lock.lock();
        relations.remove(pid_pointer_relation(pid, nullptr));
        lock.unlock();
    }
} process_list;

extern struct system_scheduler_t {
    std_k::queue<process*> run_queue;
    std_k::mutex           lock;
    bool                   paused = false;

    bool   empty() const { return run_queue.empty(); }
    size_t size() const { return run_queue.size(); }

    process* get() {
        if (empty() || paused) { return nullptr; }

        lock.lock();
        process* target = nullptr;
        if (!run_queue.empty()) {
            target = run_queue.front();
            run_queue.pop();
        }
        lock.unlock();

        return target;
    }

    void add_process(process* target) {
        lock.lock();
        run_queue.push(target);
        lock.unlock();
    }
} system_scheduler;

struct thread_scheduler {
    logical_core* owner;

    process* current_task;

#define THREAD_TIMER_DEFAULT_RATE 10000 // 100 microsecond interval / 10 kHz
#define SCHEDULING_DEFAULT_RATE   100   // 10 millisecond interval / 100 Hz
#define SCHEDULING_DEFAULT_PERIOD (double)(1 / SCHEDULING_DEFAULT_RATE)
    apic<>* local_timer;

    std_k::preset_function<void(thread_scheduler*, general_regs_state*,
                                interrupt_frame*)>
                  scheduling_function;
    apic<>::task* scheduling_timer_task = nullptr;

    thread_scheduler() {}
    thread_scheduler(logical_core* owner)
        : owner(owner)
        , current_task(nullptr)
        , local_timer(&owner->local_apic)
        , scheduling_function(run, this, 0, 0) {}

    void enter_sleep() {
        // Clear task and setup scheduling timer
        current_task = nullptr;

        scheduling_timer_task = local_timer->push_task_rate(
            SCHEDULING_DEFAULT_RATE, &scheduling_function, 1);

        enable_interrupts();

        while (1) { asm volatile("hlt"); }
    }

    bool in_sleep() { return (current_task == nullptr); }

    static void run(thread_scheduler* target, general_regs_state* task_regs,
                    interrupt_frame* frame);

    void yield_current(general_regs_state* task_regs, interrupt_frame* frame);
};

} // namespace threading

#endif

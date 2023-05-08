#ifndef PINTOS_APIC_H
#define PINTOS_APIC_H

#include "interrupts/interface.h"
#include "interrupts/interrupt_tree.h"
#include "libk/asm.h"
#include "libk/common.h"
#include "libk/heap.h"
#include "libk/mutex.h"
#include "time/timable_device.h"

struct apic_id {

    uint32_t id;

    static inline uint8_t core_bits   = 0;
    static inline uint8_t thread_bits = 0;

    apic_id(uint32_t new_id)
        : id(new_id) {}

    operator uint32_t() { return id; }

    inline unsigned int core_index() {
        return ((id >> thread_bits) & ((1 << core_bits) - 1));
    }

    inline unsigned int thread_index() {
        return (id & ((1 << thread_bits) - 1));
    }
};

namespace current_apic {
extern volatile uint32_t* apic_base;

inline volatile uint32_t& get_register(uint_fast16_t offset) {
    return *((volatile uint32_t*)((uintptr_t)current_apic::apic_base + offset));
}

inline apic_id get_id() { return ((get_register(0x20) >> 24) & 0xff); };

inline void send_apic_command(uint32_t destination, uint8_t vector,
                              uint8_t mode = 0, bool logical_dest = false,
                              bool de_assert = false, uint8_t dest_type = 0) {
    get_register(0x310)
        = (get_register(0x310) & ~(0xff << 24)) | ((destination & 0xff) << 24);
    get_register(0x300)
        = (get_register(0x300) & ~(0x000cdfff)) | vector | ((mode & 0b111) << 8)
          | ((logical_dest ? 1 : 0) << 11) | ((de_assert ? 0b10 : 0b01) << 14)
          | ((dest_type & 2) << 18);

    // Wait for delivery
    while (get_register(0x300) & (1 << 12)) {}
};

uint32_t determine_apic_tick_rate();
} // namespace current_apic

template<bool oneshot = true, bool periodic = false>
class apic
    : public timable_device<oneshot, periodic>
    , public virtual device {
  public:
    static constexpr uint8_t target_irq = 0xa0;

    static constexpr const char* default_name  = "apic";
    static constexpr const char* default_model = "APIC";
    static constexpr const char* default_path  = "/";

    apic()
        : device(default_name, default_model, nullptr, 0)
        , id(current_apic::get_id())
        , apic_rate(current_apic::determine_apic_tick_rate()) {
        interrupts::vector_override(target_irq, &int_tree_node);
        if constexpr (oneshot && !periodic) {
            // Oneshot mode
            current_apic::get_register(0x320) = target_irq & (~(1 << 17));
        } else if constexpr (!oneshot && periodic) {
            // Periodic mode
            current_apic::get_register(0x320) = target_irq | (1 << 17);
        }

        // Enable this APIC
        write_msr(0x1b, (read_msr(0x1b) | (1 << 11)));
        current_apic::get_register(0x0f0) = 0x1ff;

        // Register device
        devices::register_device(this, "/", &devices::device_tree);
    }

    apic_id  id;
    uint64_t apic_rate;
    using timestamp = typename timable_device<oneshot, periodic>::timestamp;
    using task      = typename timable_device<oneshot, periodic>::task;

  protected:
    timestamp    total_time = 0;
    std_k::mutex lock;

    struct task_pointer_less {
        task_pointer_less() {}
        bool operator()(task* left, task* right) { return *left < *right; }
    };
    std_k::heap<task*, task_pointer_less> tasks;
    task*                                 active = nullptr;

    interrupts::interrupt_tree_node int_tree_node = this;

    void push_task(task* new_task) {
        lock.lock();
        // Check if we need to swap out active task
        if (active == nullptr) {
            active = new_task;

            set_interrupt_relative(new_task->interval);
        } else if (active->time > new_task->time) {
            // Swap tasks then adjust comparator
            if (active->time != ~0UL) tasks.push(active);
            active = new_task;

            // Need to adjust for previous task getting canceled early
            timestamp interval
                = (total_time - time_to_next()) + new_task->interval;
            total_time = total_time + interval;
            set_interrupt_relative(interval);
        } else {
            // Just add to heap
            tasks.push(new_task);
        }

        lock.unlock();
    }

  public:
    int num_tasks() const { return tasks.get_vec().size(); }

    // region Template conversions
    template<bool to_o, bool to_p> operator apic<to_o, to_p>() {
        apic<to_o, to_p> to_apic;
        to_apic.id         = id;
        to_apic.apic_rate  = apic_rate;
        to_apic.total_time = total_time;
        to_apic.tasks      = tasks;
        to_apic.active     = active;

        return to_apic;
    }
    // endregion

    // region Timable device functions
    timestamp now() const override {
        // Total time is increased when each interrupt is set, so the actual
        // current time has to subtract the remaining time.
        return total_time - time_to_next();
    }

    timestamp time_to_next() const override {
        return (timestamp)current_apic::get_register(0x390);
    }
    timestamp convert_sec(double seconds) const override {
        return static_cast<timestamp>(seconds * static_cast<double>(apic_rate));
    }
    timestamp convert_rate(unsigned long rate) const override {
        return apic_rate / rate;
    }

    void set_interrupt_relative(timestamp offset) override {
        if constexpr (!oneshot) return;
        if constexpr (periodic) {
            // Oneshot mode
            current_apic::get_register(0x320)
                = current_apic::get_register(0x320) & (~(1 << 17));
        }

        // Value
        current_apic::get_register(0x380) = (offset & ((uint32_t)~0));
    };

    void set_interrupt_absolute(timestamp absolute) override {
        if constexpr (!oneshot) return;

        uint32_t offset = (absolute - now()) & ((uint32_t)~0);
        if constexpr (periodic) {
            // Oneshot mode
            current_apic::get_register(0x320)
                = current_apic::get_register(0x320) & (~(1 << 17));
        }

        // Value
        current_apic::get_register(0x380) = offset;
    };

    void set_interrupt_periodic(timestamp interval) override {
        if constexpr (!periodic) return;
        if constexpr (oneshot) {
            // Periodic mode
            current_apic::get_register(0x320)
                = current_apic::get_register(0x320) | (1 << 17);
        }

        // Value
        current_apic::get_register(0x380) = (interval & ((uint32_t)~0));
    };

    // endregion

    // region Timer functions
    task* push_task_sec(double seconds, std_k::callable<void>* to_call,
                        int rounds) override {
        timestamp interval = convert_sec(seconds);
        task* new_task = new task(to_call, interval, rounds, now() + interval);
        push_task(new_task);
        return new_task;
    }
    task* push_task_rate(unsigned long rate, std_k::callable<void>* to_call,
                         int rounds) override {
        timestamp interval = convert_rate(rate);
        task* new_task = new task(to_call, interval, rounds, now() + interval);
        push_task(new_task);
        return new_task;
    }
    task* push_task_interval(unsigned long          interval,
                             std_k::callable<void>* to_call,
                             int                    rounds) override {
        task* new_task = new task(to_call, interval, rounds, now() + interval);
        push_task(new_task);
        return new_task;
    }

    void run() override {
        if constexpr (!oneshot) return;

        lock.lock();

        // Active task needs to be replaced
        std_k::callable<void>* to_be_called
            = (active->rounds ? active->target : nullptr);
        if (active->rounds > 0) { active->rounds--; }
        if (active->rounds != 0) { active->time += active->interval; }
        task* old_task = active;

        // Prepare next task
        if (tasks.empty() && (active->rounds != 0)) {
            // Only task anyways
            total_time = active->time;
            set_interrupt_relative(active->interval);
        } else if (tasks.empty()) {
        // No task to replace
        no_task:
            active = nullptr;
            set_interrupt_relative(0);

            delete old_task;
        } else if (active->rounds != 0) {
            // Need to return to heap before taking next
            task* new_task = tasks.top();
            tasks.replace_top(active);

            if (new_task->rounds == 0) {
                // Task has been cancelled, grab another
                while (new_task->rounds == 0) {
                    delete new_task;

                    if (tasks.empty()) goto no_task;
                    new_task = tasks.top();
                    tasks.pop_top();
                }
            }

            active     = new_task;
            total_time = active->time;
            set_interrupt_relative(active->interval);
        } else {
            // Just take top from heap
            task* new_task = tasks.top();
            tasks.pop_top();

            if (new_task->rounds == 0) {
                // Task has been cancelled, grab another
                while (new_task->rounds == 0) {
                    delete new_task;

                    if (tasks.empty()) goto no_task;
                    new_task = tasks.top();
                    tasks.pop_top();
                }
            }

            active     = new_task;
            total_time = active->time;
            set_interrupt_relative(active->interval);

            delete old_task;
        }
        lock.unlock();

        // Call the task, as long as it hasn't been cancelled
        if (to_be_called != nullptr) to_be_called->call();
    }
    bool empty() const override { return (active->time == ~0UL); }
    // endregion
};

#endif // PINTOS_APIC_H

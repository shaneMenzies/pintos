#ifndef PINTOS_HPET_H
#define PINTOS_HPET_H

#include "interrupts/interface.h"
#include "interrupts/interrupts.h"
#include "io/io_apic.h"
#include "libk/functional.h"
#include "libk/heap.h"
#include "libk/misc.h"
#include "system/acpi.h"
#include "timable_device.h"

template<bool oneshot, bool periodic> struct hpet_comparator;

struct hpet : public device {
    acpi::hpet_table*             acpi_table;
    volatile uint64_t*            address;
    uint64_t                      rate;
    uint64_t                      min_tick;
    bool                          legacy_capable;
    bool                          long_capable;
    unsigned int                  num_comparators;
    hpet_comparator<true, false>* comparators;

    static constexpr const char* default_name  = "hpet";
    static constexpr const char* default_model = "HPET";
    static constexpr const char* default_path  = "/";

    volatile uint64_t* get_comparator_address(int index) const {
        return (volatile uint64_t*)((uintptr_t)address + 0x100
                                    + (0x20 * index));
    };

    volatile uint64_t& main_counter() { return address[0x1e]; }

    hpet(acpi::hpet_table* table);
};

template<bool oneshot, bool periodic>
struct hpet_comparator
    : public timable_device<oneshot, periodic>
    , public virtual device {

  public:
    using timestamp = typename timable_device<oneshot, periodic>::timestamp;
    using task      = typename timable_device<oneshot, periodic>::task;

    static constexpr const char* default_name  = "timer";
    static constexpr const char* default_model = "HPET_comparator";
    static constexpr const char* default_path  = "/hpet0";

    hpet_comparator() {}
    hpet_comparator(hpet* hpet_addr, unsigned int comparator)
        : device(default_name, default_model, nullptr, 0)
        , parent(hpet_addr)
        , target(hpet_addr->get_comparator_address(comparator))
        , index(comparator)
        , periodic_capable((target[0] & (1 << 4)) ? true : false)
        , int_target(run, this)
        , valid_irqs((target[0] >> 32) & 0xffffffff)
        , current_irq((target[0] >> 9) & 0xb11111)
        , used(false) {

        if (index == 0) {
            // Replacing the PIT, so disable it
            uint8_t disable_command = ((0) << 6) | ((0b11) << 4);
            out_byte(disable_command, IO_ports::PIT_CMD);
        }

        // Determine best IRQ Line
        // Note: It is possible for multiple comparators to be limited to the
        // same line
        int      next_irq    = 0;
        uint32_t tested_irqs = valid_irqs;
        while ((next_irq < 32) && !(tested_irqs & (1 << next_irq))) {
            next_irq++;
        }
        unsigned int pic_index = 0;
        if (index == 0) {
            // Override PIT vector
            current_vector = interrupts::vector_override(interrupts::IRQ_BASE,
                                                         &int_tree_node);
        } else {
            current_vector = interrupts::vector_alloc(&int_tree_node);
        }
        device* target_device = devices::find_device("/pic", pic_index);
        while (1) {
            if (std_k::strcmp(target_device->model, io_apic::default_model) == 0
                && next_irq >= ((io_apic*)target_device)->first_irq
                && next_irq <= ((io_apic*)target_device)->last_irq) {

                io_apic* target_apic = (io_apic*)target_device;

                // Correct irq, check if available
                if (!target_apic->irq_used(next_irq)
                    || (index == 0
                        && target_apic->irq_to_vector(next_irq)
                               == current_vector)) {
                    // Is available, take this slot
                    target_apic->set_irq(next_irq, current_vector,
                                         &int_tree_node, true);
                    valid       = true;
                    current_irq = next_irq;
                    target[0]   = (target[0] & ~(0b11111 << 9))
                                | ((current_irq & 0b11111) << 9);
                    break;
                } else {
                    // Not available, find next irq
                    tested_irqs ^= (1 << next_irq);
                    while ((next_irq < 32)
                           && !(tested_irqs & (1 << next_irq))) {
                        next_irq++;
                    }

                    if (next_irq >= 32) {
                        // None of the valid irqs are free
                        interrupts::vector_free(current_vector);
                        valid = false;
                        break;
                    }
                }
            } else {
                // Incorrect apic, check next (if it exists)
                pic_index++;
                target_device = devices::find_device("/pic", pic_index);

                if (target_device == nullptr) {
                    // No more apics, no available line for this hpet.
                    interrupts::vector_free(current_vector);
                    valid = false;
                    break;
                }
            }
        }

        // Set active task to none, and set timing mode
        active.time = ~(0UL);
        if constexpr (oneshot & !periodic) {
            target[0] = (target[0] & ~0b1000);
        }

        if (valid) {
            // Route correct interrupt
            interrupts::set_interrupt(current_vector, &int_target);

            // Clear value
            set_comparator_value(0);
            // Enable interrupt
            target[0] = target[0] | (1 << 2);
        }

        // Register device
        devices::register_device(this, default_path, &devices::device_tree);
    }

  protected:
    hpet*                           parent;
    volatile uint64_t*              target;
    unsigned int                    index;
    bool                            periodic_capable;
    interrupts::interrupt_tree_node int_tree_node = this;

    std_k::preset_function<void(hpet_comparator*)> int_target;

    std_k::min_heap<task> tasks;
    task                  active;

    void set_comparator_value(uint64_t value) { target[1] = value; }
    void push_task(task new_task) {
        // Check if we need to swap out active task
        if (active.time > new_task.time) {
            // Swap tasks then adjust comparator
            if (active.time != ~0UL) tasks.push(active);
            active = new_task;

            set_interrupt_absolute(active.time);
        } else {
            // Just add to heap
            tasks.push(new_task);
        }
    }

  public:
    uint32_t valid_irqs;
    uint8_t  current_irq;
    uint8_t  current_vector;
    bool     valid;
    bool     used;

    // region template conversions
    template<bool to_o, bool to_p> operator hpet_comparator<to_o, to_p>() {
        hpet_comparator<to_o, to_p> to_comparator(parent, index);
        to_comparator.tasks  = tasks;
        to_comparator.active = active;

        return to_comparator;
    }
    // endregion

    // region timable_device
    timestamp now() const override { return parent->main_counter(); }
    timestamp time_to_next() const override {
        return target[1] - parent->main_counter();
    }

    timestamp convert_sec(double seconds) const override {
        return static_cast<timestamp>(static_cast<double>(parent->rate)
                                      * seconds);
    }
    timestamp convert_rate(uint64_t rate) const override {
        return (parent->rate / rate);
    }

    void set_interrupt_relative(timestamp offset) override {
        if constexpr (!oneshot) return;
        if constexpr (periodic) target[0] = (target[0] & 0b1000);

        set_comparator_value((now() + offset));
    }

    void set_interrupt_absolute(timestamp absolute) override {
        if constexpr (!oneshot) return;
        if constexpr (periodic) target[0] = (target[0] & 0b1000);

        set_comparator_value(absolute);
    }

    void set_interrupt_periodic(timestamp interval) override {
        if constexpr (!periodic) return;

        target[0] = (target[0] & 0b1001000);
        set_comparator_value(now() + interval);
        set_comparator_value((interval));
    }
    // endregion

    // region timer
    void push_task_sec(double seconds, std_k::callable<void>* to_call,
                       int rounds) override {
        timestamp interval = convert_sec(seconds);
        push_task(task(to_call, interval, rounds, now() + interval));
    }
    void push_task_rate(unsigned long rate, std_k::callable<void>* to_call,
                        int rounds) override {
        timestamp interval = convert_rate(rate);
        push_task(task(to_call, interval, rounds, now() + interval));
    }
    void push_task_interval(unsigned long          interval,
                            std_k::callable<void>* to_call,
                            int                    rounds) override {
        push_task(task(to_call, interval, rounds, now() + interval));
    }

    void run() override {
        if constexpr (!oneshot) return;

        // Active task needs to be replaced
        if (active.rounds > 0) { active.rounds--; }
        if (active.rounds != 0) { active.time += active.interval; }
        std_k::callable<void>* to_be_called = active.target;

        // Prepare next task
        if (tasks.empty() && (active.rounds != 0)) {
            // Only task anyways
            set_interrupt_absolute(active.time);
        } else if (tasks.empty()) {
            // No task to replace
            active.time = ~0;
            set_interrupt_absolute(0);
        } else if (active.rounds != 0) {
            // Need to return to heap before taking next
            task saved = tasks.top();
            tasks.replace_top(active);

            active = saved;
            set_interrupt_absolute(active.time);
        } else {
            // Just take top from heap
            active = tasks.top();
            tasks.pop_top();
            set_interrupt_absolute(active.time);
        }

        // Call the task
        to_be_called->call();
    }
    static void run(hpet_comparator<oneshot, periodic>* target) {
        target->run();
    }
    bool empty() const override { return (active.time == ~0UL); }
    // endregion
};

#endif // PINTOS_HPET_H

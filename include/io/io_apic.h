#ifndef PINTOS_IO_APIC_H
#define PINTOS_IO_APIC_H

#include "device/device.h"
#include "interrupts/interface.h"
#include "interrupts/interrupt_tree.h"
#include "libk/common.h"
#include "system/acpi.h"
#include "threading/apic.h"

struct io_apic : public device {
    volatile uint32_t* target;
    uint8_t            num_irqs;
    uint8_t            first_irq;
    uint8_t            last_irq;
    uint8_t            id;

    static constexpr const char* default_name  = "pic";
    static constexpr const char* default_model = "I/O APIC";
    static constexpr const char* default_path  = "/";

    struct irq_entry {
        enum delivery_mode_type {
            fixed   = 0b000,
            lowest  = 0b001,
            smi     = 0b010,
            nmi     = 0b100,
            init    = 0b101,
            ext_int = 0b111
        };
        enum dest_mode_type { physical = 0, logical = 1 };
        enum status_type {
            idle    = 0,
            pending = 1,
        };
        enum polarity_type { active_high = 0, active_low = 1 };
        enum trigger_mode_type { edge = 0, level = 1 };
        enum mask_type { enabled = 0, disabled = 1 };
        union {
            uint64_t raw;
            struct {
                uint64_t           int_vector : 8;
                delivery_mode_type delivery_mode : 3;
                dest_mode_type     dest_mode : 1;
                status_type        status : 1;
                polarity_type      polarity : 1;
                uint64_t           irr : 1;
                trigger_mode_type  trigger_mode : 1;
                mask_type          masked : 1;
                uint64_t           reserved : 39;
                uint64_t           destination : 8;
            } __attribute__((packed));
        };
        interrupts::interrupt_tree_node* owner;

        irq_entry()
            : int_vector(0)
            , delivery_mode(fixed)
            , dest_mode(physical)
            , polarity(active_high)
            , trigger_mode(edge)
            , masked(disabled)
            , destination(0)
            , owner(nullptr) {}
        irq_entry(uint64_t value)
            : raw(value)
            , owner(nullptr) {}
        irq_entry(uint64_t value, interrupts::interrupt_tree_node* owner)
            : raw(value)
            , owner(owner) {}
    };

    io_apic(acpi::madt_io_apic* source, acpi::madt_table* madt);

    irq_entry get_irq(uint8_t index) {
        reg_select()  = (index * 2) + 0x10;
        uint32_t low  = reg_data();
        reg_select()  = (index * 2) + 0x11;
        uint32_t high = reg_data();

        return irq_entry((uint64_t)(low | ((uint64_t)high << 32)),
                         int_tree_node.children[index]);
    }
    void set_irq(uint8_t index, irq_entry entry) {
        reg_select()                  = (index * 2) + 0x10;
        reg_data()                    = (entry.raw & (uint32_t)(~0));
        reg_select()                  = (index * 2) + 0x11;
        reg_data()                    = ((entry.raw >> 32) & (uint32_t)(~0));
        int_tree_node.children[index] = entry.owner;
        current_mapping[index]        = entry.int_vector & 0xff;
        const int path                = -1;
        interrupts::register_int_route(&int_tree_node, &path, entry.int_vector);
    }
    void set_irq(uint8_t index, uint8_t vector) {
        reg_select() = (index * 2) + 0x10;
        interrupts::interrupt_tree.root->children[reg_data() & 0xff] = nullptr;
        reg_data()             = (reg_data() & ~0xff) | vector;
        current_mapping[index] = vector;
        const int path         = -1;
        interrupts::register_int_route(&int_tree_node, &path, vector);
    }
    void set_irq(uint8_t index, bool enabled) {
        reg_select() = (index * 2) + 0x10;
        reg_data()   = (reg_data() & ~(1 << 16))
                     | ((enabled ? irq_entry::mask_type::enabled
                                 : irq_entry::mask_type::disabled)
                        << 16);
    }
    void set_irq(uint8_t index, uint8_t vector,
                 interrupts::interrupt_tree_node* owner, bool enabled) {
        reg_select() = (index * 2) + 0x10;
        interrupts::interrupt_tree.root->children[reg_data() & 0xff] = nullptr;
        reg_data() = (reg_data() & ~((1 << 16) | 0xff))
                     | ((enabled ? irq_entry::mask_type::enabled
                                 : irq_entry::mask_type::disabled)
                        << 16)
                     | vector;
        int_tree_node.children[index] = owner;
        current_mapping[index]        = vector;
        const int path                = -1;
        interrupts::register_int_route(&int_tree_node, &path, vector);
    }

    bool irq_used(uint8_t index) {
        return (int_tree_node.children[index] != nullptr);
    }
    uint8_t irq_to_vector(uint8_t irq) { return current_mapping[irq]; }
    uint8_t vector_to_irq(uint8_t vector) {
        for (uint8_t i = 0; i < num_irqs; i++) {
            if (current_mapping[i] == vector) return i;
        }
        return 0xff;
    }

  protected:
    interrupts::interrupt_tree_node int_tree_node;
    uint8_t*                        current_mapping;

    constexpr volatile uint32_t& reg_select() { return *target; }
    constexpr volatile uint32_t& reg_data() {
        return target[(0x10 / sizeof(volatile uint32_t))];
    }
};

#endif // PINTOS_IO_APIC_H

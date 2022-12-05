#ifndef PINTOS_LEGACY_PIC_H
#define PINTOS_LEGACY_PIC_H

#include "device/device.h"
#include "interrupts/interrupt_tree.h"
#include "io.h"
#include "libk/common.h"

/** @brief Legacy PIC's device representation
 *
 *  The Legacy PIC is very limited by comparison to the IOAPIC, the routed
 *  vectors can't be customized beyond the initial offset, so the PIC will
 *  take ownership of all of its vectors preemptively.
 */
struct legacy_pic : public device {
  public:
    static constexpr uint8_t first_irq = 0;
    static constexpr uint8_t last_irq  = 15;
    static constexpr uint8_t num_irqs  = 16;

    static constexpr const char* default_name  = "pic";
    static constexpr const char* default_model = "8259 PIC";
    static constexpr const char* default_path  = "/";

    const uint8_t vector_offset;
    legacy_pic(uint8_t offset);

    struct irq_entry {
        bool                             enabled;
        interrupts::interrupt_tree_node* owner;

        irq_entry()
            : enabled(false)
            , owner(nullptr) {}
        irq_entry(bool enabled, interrupts::interrupt_tree_node* owner)
            : enabled(enabled)
            , owner(owner) {}
    };

    irq_entry get_irq(uint8_t index) {
        return irq_entry(get_mask(index), node_children[index]);
    }
    void set_irq(uint8_t index, irq_entry entry) {
        set_mask(index, entry.enabled);
        node_children[index] = entry.owner;
    }
    void set_irq(uint8_t index, bool enabled) { set_mask(index, enabled); }

    bool irq_used(uint8_t index) { return (node_children[index] != nullptr); }
    uint8_t irq_to_vector(uint8_t irq) { return (irq + vector_offset); }
    uint8_t vector_to_irq(uint8_t vector) { return (vector - vector_offset); }

  protected:
    interrupts::interrupt_tree_node  int_tree_node;
    interrupts::interrupt_tree_node* node_children[num_irqs];

    inline void set_mask(uint8_t index, bool value) {
        if (index < 8) {
            out_byte(in_byte(IO_ports::PIC_1_DATA) | ((value ? 1 : 0) << index),
                     IO_ports::PIC_1_DATA);
        } else {
            out_byte(in_byte(IO_ports::PIC_2_DATA)
                         | ((value ? 1 : 0) << (index - 8)),
                     IO_ports::PIC_2_DATA);
        }
    }

    inline bool get_mask(uint8_t index) {
        if (index < 8) {
            return (in_byte(IO_ports::PIC_1_DATA) >> index) & 1;
        } else {
            return (in_byte(IO_ports::PIC_2_DATA) >> (index - 8)) & 1;
        }
    }
};

#endif // PINTOS_LEGACY_PIC_H

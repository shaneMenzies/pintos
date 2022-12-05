/**
 * @file legacy_pic.cpp
 * @author Shane Menzies
 * @brief
 * @date 9/8/22
 *
 *
 */

#include "legacy_pic.h"

#include "interrupts/interface.h"

const uint64_t   num_legacy_pic_properties = 1;
device::property legacy_pic_properties[num_legacy_pic_properties]
    = {{device::property::predefined::serial_number, 0}};

legacy_pic::legacy_pic(uint8_t offset)
    : device(default_name, default_model, legacy_pic_properties,
             num_legacy_pic_properties)
    , vector_offset(offset)
    , int_tree_node(this, node_children, num_irqs) {

    // Send Initialization Commands
    out_byte(0x10 | 0x1, IO_ports::PIC_1_CMD);
    out_byte(0x10 | 0x1, IO_ports::PIC_2_CMD);
    io_wait();

    // Give Vector Offsets
    out_byte(offset, IO_ports::PIC_1_DATA);
    out_byte(offset + 8, IO_ports::PIC_2_DATA);
    io_wait();

    // Setup cascading using IRQ2 on the Master
    out_byte(0b100, PIC_1_DATA);
    io_wait();
    out_byte(0b10, PIC_2_DATA);
    io_wait();

    // Put PICS in 8086 mode
    out_byte(1, PIC_1_DATA);
    out_byte(1, PIC_2_DATA);
    io_wait();

    // Default initialize IRQS
    for (uint8_t i = first_irq; i <= last_irq; i++) {
        set_irq(i, irq_entry(false, nullptr));
    }

    // IRQ 2 is used for cascading
    set_irq(2, irq_entry(false, &int_tree_node));

    // Register device in device and interrupt routings
    devices::register_device(this, default_path, &devices::device_tree);
    for (unsigned int i = first_irq; i < num_irqs; i++) {
        int path = -1;
        interrupts::register_int_route(&int_tree_node, &path, i,
                                       &interrupts::interrupt_tree);
    }
}

/**
 * @file interrupts.cpp
 * @author Shane Menzies
 * @brief Functions for handling interrupts
 * @date 03/14/21
 * 
 * 
 */

#include "interrupts.h"

namespace interrupts {

    // Main interrupt descriptor table
    x86_tables::idt_gate idt_table[256];
    const size_t IDT_SIZE = (256 * sizeof(x86_tables::idt_gate) - 1);

    void idt_init() {

        // Tell CPU where the interrupt table is
        set_idt((void*)idt_table, (uint16_t)IDT_SIZE);

        // Initialize the PICs
        out_byte(PIC_1_CMD, CODE_INIT);
        out_byte(PIC_2_CMD, CODE_INIT);
        io_wait();

        out_byte(PIC_1_DATA, OFFSET_1);
        out_byte(PIC_2_DATA, OFFSET_2);
        io_wait();

        out_byte(PIC_1_DATA, CODE_HAS_SLAVE);
        out_byte(PIC_2_DATA, CODE_IS_SLAVE);
        io_wait();

        out_byte(PIC_1_DATA, CODE_8086);
        out_byte(PIC_2_DATA, CODE_8086);
        io_wait();

        // Clear the masks
        for (uint8_t i = 0; i < 16; i++) {
            PIC_clear_mask(i);
        }
    }

    void set_interrupt(uint8_t interrupt, uint8_t gate_type, void (*handler)(interrupt_frame* frame)) {
        x86_tables::set_gate(idt_table[interrupt], (void*)handler, 0x8, 0b11, 
                             gate_type, true, false);

        // Update PIC mask if this handler is in the PIC range
        if (interrupt >= OFFSET_1 && interrupt < OFFSET_END) {
            PIC_set_mask((interrupt - OFFSET_1));
        }
    }

    void set_interrupt(uint8_t interrupt, uint8_t gate_type, void (*handler)(interrupt_frame* frame, unsigned int error_code)) {
        x86_tables::set_gate(idt_table[interrupt], (void*)handler, 0x8, 0b11, 
                             gate_type, true, false);

        // Update PIC mask if this handler is in the PIC range
        if (interrupt >= OFFSET_1 && interrupt < OFFSET_END) {
            PIC_set_mask((interrupt - OFFSET_1));
        }
    }

    void clear_interrupt(uint8_t interrupt) {
        x86_tables::set_gate(idt_table[interrupt], 0, 0x8, 0b11, 
                             0, false, false);

        // Update PIC mask if this handler is in the PIC range
        if (interrupt >= OFFSET_1 && interrupt < OFFSET_END) {
            PIC_clear_mask((interrupt - OFFSET_1));
        }
    }

    void PIC_EOI(uint8_t IRQ) {
        if (IRQ >= 8) 
            out_byte(PIC_2_CMD, CODE_EOI);
        out_byte(PIC_1_CMD, CODE_EOI);
    }

    void PIC_clear_mask(uint8_t IRQ) {
        uint16_t port = PIC_1_DATA;
        if (IRQ >= 8) {
            port = PIC_2_DATA;
            IRQ -= 8;
        }

        uint8_t value = in_byte(port) | (1 << IRQ);
        out_byte(port, value);
    }

    void PIC_set_mask(uint8_t IRQ) {
        uint16_t port = PIC_1_DATA;
        if (IRQ >= 8) {
            port = PIC_2_DATA;
            IRQ -= 8;
        }

        uint8_t value = in_byte(port) & ~(1 << IRQ);
        out_byte(port, value);
    }


    

}

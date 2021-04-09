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

    /**
     * @brief Initializes the Interrupt Descriptor Table
     * 
     */
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

    /**
     * @brief Initializes the default interrupt handlers
     * 
     */
    void interrupts_init() {

        // Set up exception handlers
        interrupts::set_interrupt(0, TRAP_GATE_32, interrupts::div_zero);
        interrupts::set_interrupt(1, TRAP_GATE_32, interrupts::debug);
        interrupts::set_interrupt(2, TRAP_GATE_32, interrupts::nmi);
        interrupts::set_interrupt(3, TRAP_GATE_32, interrupts::breakpoint);
        interrupts::set_interrupt(4, TRAP_GATE_32, interrupts::overflow);
        interrupts::set_interrupt(5, TRAP_GATE_32, interrupts::bound_range);
        interrupts::set_interrupt(6, TRAP_GATE_32, interrupts::invalid_opcode);
        interrupts::set_interrupt(7, TRAP_GATE_32, interrupts::device_naval);
        interrupts::set_interrupt(8, TRAP_GATE_32, interrupts::double_fault);
        interrupts::set_interrupt(10, TRAP_GATE_32, interrupts::segment_fault);
        interrupts::set_interrupt(11, TRAP_GATE_32, interrupts::segment_fault);
        interrupts::set_interrupt(12, TRAP_GATE_32, interrupts::segment_fault);
        interrupts::set_interrupt(13, TRAP_GATE_32, interrupts::gpf_handler);
        interrupts::set_interrupt(14, TRAP_GATE_32, interrupts::page_fault);
        interrupts::set_interrupt(16, TRAP_GATE_32, interrupts::flt_x87);
        interrupts::set_interrupt(17, TRAP_GATE_32, interrupts::unaligned_mem);
        interrupts::set_interrupt(19, TRAP_GATE_32, interrupts::flt_x87);

        // Hardware IRQs
        interrupts::set_interrupt((OFFSET_1 + 0), INT_GATE_32, interrupts::irq_0);
        interrupts::set_interrupt((OFFSET_1 + 1), INT_GATE_32, interrupts::irq_1);
        interrupts::set_interrupt((OFFSET_1 + 2), INT_GATE_32, interrupts::irq_2);
        interrupts::set_interrupt((OFFSET_1 + 3), INT_GATE_32, interrupts::irq_3);
        interrupts::set_interrupt((OFFSET_1 + 4), INT_GATE_32, interrupts::irq_4);
        interrupts::set_interrupt((OFFSET_1 + 5), INT_GATE_32, interrupts::irq_5);
        interrupts::set_interrupt((OFFSET_1 + 6), INT_GATE_32, interrupts::irq_6);
        interrupts::set_interrupt((OFFSET_1 + 7), INT_GATE_32, interrupts::irq_7);
        interrupts::set_interrupt((OFFSET_1 + 8), INT_GATE_32, interrupts::irq_8);
        interrupts::set_interrupt((OFFSET_1 + 9), INT_GATE_32, interrupts::irq_9);
        interrupts::set_interrupt((OFFSET_1 + 10), INT_GATE_32, interrupts::irq_10);
        interrupts::set_interrupt((OFFSET_1 + 11), INT_GATE_32, interrupts::irq_11);
        interrupts::set_interrupt((OFFSET_1 + 12), INT_GATE_32, interrupts::irq_12);
        interrupts::set_interrupt((OFFSET_1 + 13), INT_GATE_32, interrupts::irq_13);
        interrupts::set_interrupt((OFFSET_1 + 14), INT_GATE_32, interrupts::irq_14);
        interrupts::set_interrupt((OFFSET_1 + 15), INT_GATE_32, interrupts::irq_15);

        // Set up keyboard interrupt
        //interrupts::set_interrupt((OFFSET_1 + 1), INT_GATE_32, interrupts::keyboard_handler);
    }

    /**
     * @brief Sets an interrupt handler in the interrupt table, and enables
     *        it's corresponding hardware interrupt on the PIC if applicable.
     * 
     * @param interrupt     Number of target interrupt
     * @param gate_type     Gate type for the interrupt to be set as
     * @param handler       New interrupt handler
     */
    void set_interrupt(uint8_t interrupt, uint8_t gate_type, void (*handler)(interrupt_frame* frame)) {
        x86_tables::set_gate(idt_table[interrupt], (void*)handler, 0x8, 0b11, 
                             gate_type, true, false);

        // Update PIC mask if this handler is in the PIC range
        if (interrupt >= OFFSET_1 && interrupt < OFFSET_END) {
            PIC_set_mask((interrupt - OFFSET_1));
        }
    }

    /**
     * @brief Sets an interrupt handler in the interrupt table, and enables
     *        it's corresponding hardware interrupt on the PIC if applicable.
     * 
     * @param interrupt     Number of target interrupt
     * @param gate_type     Gate type for the interrupt to be set as
     * @param handler       New interrupt handler
     */
    void set_interrupt(uint8_t interrupt, uint8_t gate_type, void (*handler)(interrupt_frame* frame, unsigned int error_code)) {
        x86_tables::set_gate(idt_table[interrupt], (void*)handler, 0x8, 0b11, 
                             gate_type, true, false);

        // Update PIC mask if this handler is in the PIC range
        if (interrupt >= OFFSET_1 && interrupt < OFFSET_END) {
            PIC_set_mask((interrupt - OFFSET_1));
        }
    }

    /**
     * @brief Clears the corresponding handler for a certain interrupt
     * 
     * @param interrupt     Number of target interrupt
     */
    void clear_interrupt(uint8_t interrupt) {
        x86_tables::set_gate(idt_table[interrupt], 0, 0x8, 0b11, 
                             0, false, false);

        // Update PIC mask if this handler is in the PIC range
        if (interrupt >= OFFSET_1 && interrupt < OFFSET_END) {
            PIC_clear_mask((interrupt - OFFSET_1));
        }
    }

    /**
     * @brief Sends the End Of Interrupt signal to the pics for the 
     *        given IRQ trigger
     * 
     * @param IRQ   Hardware interrupt to end (0-15 IRQ = 32-47 INT)
     */
    void PIC_EOI(uint8_t IRQ) {
        if (IRQ >= 8) 
            out_byte(PIC_2_CMD, CODE_EOI);
        out_byte(PIC_1_CMD, CODE_EOI);
    }

    /**
     * @brief Disables a hardware interrupt in the PIC mask
     * 
     * @param IRQ   Hardware interrupt to end (0-15 IRQ = 32-47 INT)
     */
    void PIC_clear_mask(uint8_t IRQ) {
        uint16_t port = PIC_1_DATA;
        if (IRQ >= 8) {
            port = PIC_2_DATA;
            IRQ -= 8;
        }

        uint8_t value = in_byte(port) | (1 << IRQ);
        out_byte(port, value);
    }

    /**
     * @brief Enables a hardware interrupt in the PIC mask
     * 
     * @param IRQ   Hardware interrupt to end (0-15 IRQ = 32-47 INT)
     */
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

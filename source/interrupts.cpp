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
    bool legacy_pic_mode = true;

    int io_apic_count = 0;
    io_apic_info system_io_apic[8];

    // Mappings from IRQ sources to global interrupt mappings (used by ioapics)
    uint8_t irq_mapping[0x100];

    const uint8_t IRQ_BASE = 0x20;
    uint8_t IRQ_END = IRQ_BASE + 0x10;

    /**
     * @brief Initializes the Interrupt Descriptor Table
     * 
     */
    void idt_init(acpi::madt_table* madt) {

        // Load the new Interrupt Descriptor Table
        set_idt(idt_table, IDT_SIZE);
        
        // Check if the CPU has a built-in APIC
        uint32_t edx, unused;
        if (__get_cpuid(0x1, &unused, &unused, &unused, &edx) && (edx & (1 << 9))) {

            // Does have APIC
            legacy_pic_mode = false;
        
            // Ensure that it is enabled
            write_msr(0x1b, (read_msr(0x1b) | (1<<11)));

            // Identity map the memory taken by the APIC
            paging::identity_map_region((uintptr_t)get_apic_base(), 0x3f0);

            // Set Spurious Interrupt Vector Register(0x0f0) to interrupt #0xff
            //      0x100 to set bit 8, which enables the receiving of interrupts
            *(get_apic_register(0x0f0)) = 0x1ff;
        }


        // Get info on any IOAPICS
        acpi::madt_entry** entries = (acpi::madt_entry**)malloc(sizeof(void*) * 256);
        io_apic_count = acpi::get_madt_entries(madt, acpi::madt_entry_type::io_apic, entries, 256);
        if (io_apic_count) {
            // Process the info on the io_apics themselves, and mask all interrupts
            for (int i = 0; i < io_apic_count; i++) {
                paging::identity_map_region(((acpi::madt_io_apic*)entries[i])->io_apic_addr, sizeof(io_apic));
                system_io_apic[i].fill((acpi::madt_io_apic*)(entries[i]));
                for (uint8_t next_int = 0; next_int < system_io_apic[i].num_ints; next_int++) {
                    system_io_apic[i].address->set_masked(next_int, true);
                }
            }

            // Fill out default irq mappings
            for (int i = 0; i < 0x100; i++) {
                irq_mapping[i] = i;
            }

            // Get any source overrides
            int entry_count = acpi::get_madt_entries(madt, acpi::madt_entry_type::io_apic_src_override, entries, 256);
            for (int i = 0; i < entry_count; i++) {
                uint8_t irq_source = ((acpi::madt_io_apic_src_override*)entries[i])->irq_src;
                uint8_t int_source = ((acpi::madt_io_apic_src_override*)entries[i])->global_int & 0xff;
                uint32_t flags = ((acpi::madt_io_apic_src_override*)entries[i])->flags;
                irq_mapping[int_source] = irq_source;
                irq_mapping[irq_source] = int_source;

                // Set flags info in the appropriate io_apic entry
                io_apic_info current_apic;
                bool valid_apic = false;
                for (int i = 0; i < io_apic_count; i++) {
                    current_apic = system_io_apic[i];
                    if (current_apic.start <= int_source && current_apic.end > int_source) {
                        valid_apic = true;
                        break;
                    }
                }
                if (valid_apic) {
                    int_source -= current_apic.start;
                    io_apic_redirection_entry target = current_apic.address->get_interrupt(int_source);
                    target.raw_data &= ~((1<<13) | (1<<15));
                    target.raw_data |= ((flags & 2) ? (1<<13) : 0);
                    target.raw_data |= ((flags & 8) ? (1<<15) : 0);
                    current_apic.address->set_interrupt(int_source, target);
                }
            }
        }
        free(entries);

        // Initialize the PICs
        out_byte(CODE_INIT, PIC_1_CMD);
        out_byte(CODE_INIT, PIC_2_CMD);
        io_wait();

        if (legacy_pic_mode) {
            // Legacy PICs need to be set up to be used
            out_byte(OFFSET_1, PIC_1_DATA);
            out_byte(OFFSET_2, PIC_2_DATA);
            io_wait();
        } else {
            // Legacy PICS need to be functionally disabled if they exist
            out_byte(0xf8, PIC_1_DATA);
            out_byte(0xf8, PIC_2_DATA);
            io_wait();
        }

        out_byte(CODE_HAS_SLAVE, PIC_1_DATA);
        out_byte(CODE_IS_SLAVE, PIC_2_DATA);
        io_wait();

        out_byte(CODE_8086, PIC_1_DATA);
        out_byte(CODE_8086, PIC_2_DATA);
        io_wait();

        // Clear the masks
        for (uint8_t i = 0; i < 16; i++) {
            PIC_clear_mask(i);
        }

    }

    /**
     * @brief Initializes the default interrupt handlers
     * 
     * @param madt  Pointer to acpi MADT table
     */
    void interrupts_init(acpi::madt_table* madt) {

        // Set up IDT and PIC/APIC
        idt_init(madt);

        // Set up exception handlers
        set_interrupt(0, TRAP_GATE_32, div_zero);
        set_interrupt(1, TRAP_GATE_32, debug);
        set_interrupt(2, TRAP_GATE_32, nmi);
        set_interrupt(3, TRAP_GATE_32, breakpoint);
        set_interrupt(4, TRAP_GATE_32, overflow);
        set_interrupt(5, TRAP_GATE_32, bound_range);
        set_interrupt(6, TRAP_GATE_32, invalid_opcode);
        set_interrupt(7, TRAP_GATE_32, device_naval);
        set_interrupt(8, TRAP_GATE_32, double_fault);
        set_interrupt(10, TRAP_GATE_32, segment_fault);
        set_interrupt(11, TRAP_GATE_32, segment_fault);
        set_interrupt(12, TRAP_GATE_32, segment_fault);
        set_interrupt(13, TRAP_GATE_32, gpf_handler);
        set_interrupt(14, TRAP_GATE_32, page_fault);
        set_interrupt(16, TRAP_GATE_32, flt_x87);
        set_interrupt(17, TRAP_GATE_32, unaligned_mem);
        set_interrupt(19, TRAP_GATE_32, flt_x87);

        if (legacy_pic_mode) {
            // Hardware IRQs for legacy PIC mode
            set_interrupt((OFFSET_1 + 0), INT_GATE_32, irq_0);
            set_interrupt((OFFSET_1 + 1), INT_GATE_32, irq_1);
            //set_interrupt((OFFSET_1 + 2), INT_GATE_32, irq_2);
            //set_interrupt((OFFSET_1 + 3), INT_GATE_32, irq_3);
            //set_interrupt((OFFSET_1 + 4), INT_GATE_32, irq_4);
            //set_interrupt((OFFSET_1 + 5), INT_GATE_32, irq_5);
            //set_interrupt((OFFSET_1 + 6), INT_GATE_32, irq_6);
            //set_interrupt((OFFSET_1 + 7), INT_GATE_32, irq_7);
            //set_interrupt((OFFSET_1 + 8), INT_GATE_32, irq_8);
            //set_interrupt((OFFSET_1 + 9), INT_GATE_32, irq_9);
            //set_interrupt((OFFSET_1 + 10), INT_GATE_32, irq_10);
            //set_interrupt((OFFSET_1 + 11), INT_GATE_32, irq_11);
            //set_interrupt((OFFSET_1 + 12), INT_GATE_32, irq_12);
            //set_interrupt((OFFSET_1 + 13), INT_GATE_32, irq_13);
            //set_interrupt((OFFSET_1 + 14), INT_GATE_32, irq_14);
            //set_interrupt((OFFSET_1 + 15), INT_GATE_32, irq_15);
        } else {
            // Hardware IRQs for the IOAPIC
            set_interrupt((IRQ_BASE + 0), INT_GATE_32, irq_0);
            set_interrupt((IRQ_BASE + 1), INT_GATE_32, irq_1);
            //set_interrupt((IRQ_BASE + 2), INT_GATE_32, irq_2);
            //set_interrupt((IRQ_BASE + 3), INT_GATE_32, irq_3);
            //set_interrupt((IRQ_BASE + 4), INT_GATE_32, irq_4);
            //set_interrupt((IRQ_BASE + 5), INT_GATE_32, irq_5);
            //set_interrupt((IRQ_BASE + 6), INT_GATE_32, irq_6);
            //set_interrupt((IRQ_BASE + 7), INT_GATE_32, irq_7);
            //set_interrupt((IRQ_BASE + 8), INT_GATE_32, irq_8);
            //set_interrupt((IRQ_BASE + 9), INT_GATE_32, irq_9);
            //set_interrupt((IRQ_BASE + 10), INT_GATE_32, irq_10);
            //set_interrupt((IRQ_BASE + 11), INT_GATE_32, irq_11);
            //set_interrupt((IRQ_BASE + 12), INT_GATE_32, irq_12);
            //set_interrupt((IRQ_BASE + 13), INT_GATE_32, irq_13);
            //set_interrupt((IRQ_BASE + 14), INT_GATE_32, irq_14);
            //set_interrupt((IRQ_BASE + 15), INT_GATE_32, irq_15);
        }

        // Set up Software Interrupts
        set_interrupt(51, INT_GATE_32, test_int);

        // Spurious Interrupts (0xf8 to 0xff)
        for (uint8_t i = 0xf8; i >= 0xf8; i++) {
            set_interrupt(i, INT_GATE_32, spurious_int);
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
    void set_interrupt(uint8_t interrupt, uint8_t gate_type, void (*handler)(interrupt_frame* frame)) {
        x86_tables::set_gate(idt_table[interrupt], (void*)handler, x86_tables::code_selector, 0b11, 
                             gate_type, true);

        if (legacy_pic_mode) {
            // Update PIC mask if this handler is in the PIC range
            if (interrupt >= OFFSET_1 && interrupt < OFFSET_END) {
                PIC_set_mask((interrupt - OFFSET_1));
            }
        } else if (interrupt >= IRQ_BASE && interrupt < IRQ_END) {
            // Update IOAPIC mask if in it's range
            uint8_t global_int = irq_mapping[interrupt - IRQ_BASE];
            io_apic_info current_apic;
            bool valid_apic = false;
            for (int i = 0; i < io_apic_count; i++) {
                current_apic = system_io_apic[i];
                if (current_apic.start <= global_int && current_apic.end > global_int) {
                    valid_apic = true;
                    break;
                }
            }

            if (valid_apic) {
                global_int -= current_apic.start;
                io_apic_redirection_entry target = current_apic.address->get_interrupt(global_int);
                target.data.masked = 0;
                target.data.int_vector = interrupt;
                target.data.delivery_mode = 0;
                target.data.trigger_mode = 0;
                target.data.dest_mode = 0;
                // TODO: With Multicore capability, decide which APIC/core should get the new interrupt
                target.data.destination = (*(get_apic_register(0x20)) & 0xf);

                current_apic.address->set_interrupt(global_int, target);
            }
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
    void set_interrupt(uint8_t interrupt, uint8_t gate_type, void (*handler)(interrupt_frame* frame, unsigned long int error_code)) {
        x86_tables::set_gate(idt_table[interrupt], (void*)handler, x86_tables::code_selector, 0b11, 
                             gate_type, true);

        if (legacy_pic_mode) {
            // Update PIC mask if this handler is in the PIC range
            if (interrupt >= OFFSET_1 && interrupt < OFFSET_END) {
                PIC_set_mask((interrupt - OFFSET_1));
            }
        } else {
            // Update IOAPIC mask if in it's range
        }
    }

    /**
     * @brief Clears the corresponding handler for a certain interrupt
     * 
     * @param interrupt     Number of target interrupt
     */
    void clear_interrupt(uint8_t interrupt) {
        x86_tables::set_gate(idt_table[interrupt], 0, x86_tables::code_selector, 0b11, 
                             0, false);

        if (legacy_pic_mode) {
            // Update PIC mask if this handler is in the PIC range
            if (interrupt >= OFFSET_1 && interrupt < OFFSET_END) {
                PIC_clear_mask((interrupt - OFFSET_1));
            }
        } else {
            // Update IOAPIC mask if in it's range
        }
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
        out_byte(value, port);
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
        out_byte(value, port);
    }
}

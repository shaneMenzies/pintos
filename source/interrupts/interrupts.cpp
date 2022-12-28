/**
 * @file interrupts.cpp
 * @author Shane Menzies
 * @brief Functions for handling interrupts
 * @date 03/14/21
 *
 *
 */

#include "interrupts.h"

#include "device/device.h"
#include "interrupt_redirect.h"
#include "io/io.h"
#include "io/io_apic.h"
#include "libk/asm.h"
#include "libk/macro.h"
#include "memory/addressing.h"
#include "memory/p_memory.h"
#include "memory/x86_tables.h"
#include "system/acpi.h"

namespace interrupts {

// Main interrupt descriptor table
x86_tables::idt_gate idt_table[256];
device*              idt_owner_table[256];
constexpr size_t     IDT_SIZE        = (256 * sizeof(x86_tables::idt_gate) - 1);
bool                 legacy_pic_mode = true;

void* target_apic_base = 0;

constexpr uint8_t IRQ_BASE = 0x20;
uint8_t           IRQ_END  = IRQ_BASE + 0x10;

template<unsigned int irq_index>
__attribute__((interrupt, hot, target("general-regs-only"))) void
    interrupt_redirect(interrupt_frame* frame) {
    (void)frame;
    if (irq_redirect_target[irq_index] != nullptr)
        irq_redirect_target[irq_index]->call();

    send_EOI(irq_index);
};

#define INIT_INTERRUPT_REDIRECT(val) interrupt_redirect<val>,
void (*const interrupt_redirect_for[256])(interrupt_frame*) = {
    MAP(INIT_INTERRUPT_REDIRECT, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
        14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
        50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
        68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85,
        86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102,
        103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
        117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130,
        131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144,
        145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158,
        159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172,
        173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186,
        187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200,
        201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214,
        215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228,
        229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242,
        243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255)};

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
        write_msr(0x1b, (read_msr(0x1b) | (1 << 11)));

        // Update APIC Address
        current_apic::apic_base = (volatile uint32_t*)((uintptr_t)read_msr(0x1b)
                                                       & (0x000ffffff000));

        // Identity map and allocate the memory taken by the APIC
        paging::kernel_address_space.identity_map_region(
            (uintptr_t)current_apic::apic_base, 0x3f0);

        // Set Spurious Interrupt Vector Register(0x0f0) to interrupt #0xff
        //      0x100 to set bit 8, which enables the receiving of interrupts
        current_apic::get_register(0x0f0) = 0x1ff;
    }

    // Get info on any IOAPICS
    int max_entries = acpi::count_entries(madt, acpi::madt_entry_type::io_apic);
    if (max_entries
        && max_entries < acpi::count_entries(
               madt, acpi::madt_entry_type::io_apic_src_override))
        max_entries = acpi::count_entries(
            madt, acpi::madt_entry_type::io_apic_src_override);
    acpi::entry_header** entries
        = (acpi::entry_header**)malloc(sizeof(void*) * max_entries);
    int io_apic_count
        = acpi::get_entries(madt, acpi::madt_entry_type::io_apic, entries, 256);
    if (io_apic_count) {
        // Initialize the io_apics
        for (int i = 0; i < io_apic_count; i++) {
            new io_apic((acpi::madt_io_apic*)entries[i], madt);
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
    for (uint8_t i = 0; i < 16; i++) { PIC_clear_mask(i); }
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
    set_direct_interrupt(0, TRAP_GATE_32, div_zero);
    set_direct_interrupt(1, TRAP_GATE_32, debug);
    set_direct_interrupt(2, TRAP_GATE_32, nmi);
    set_direct_interrupt(3, TRAP_GATE_32, breakpoint_handler);
    set_direct_interrupt(4, TRAP_GATE_32, overflow);
    set_direct_interrupt(5, TRAP_GATE_32, bound_range);
    set_direct_interrupt(6, TRAP_GATE_32, invalid_opcode);
    set_direct_interrupt(7, TRAP_GATE_32, device_naval);
    set_direct_interrupt(8, TRAP_GATE_32, double_fault);
    set_direct_interrupt(10, TRAP_GATE_32, segment_fault);
    set_direct_interrupt(11, TRAP_GATE_32, segment_fault);
    set_direct_interrupt(12, TRAP_GATE_32, segment_fault);
    set_direct_interrupt(13, TRAP_GATE_32, gpf_handler);
    set_direct_interrupt(14, TRAP_GATE_32, page_fault);
    set_direct_interrupt(16, TRAP_GATE_32, flt_x87);
    set_direct_interrupt(17, TRAP_GATE_32, unaligned_mem);
    set_direct_interrupt(19, TRAP_GATE_32, flt_x87);

    // Default to interrupt redirects
    for (int i = 20; i < 256; i++) {
        set_direct_interrupt(i & 0xff, INT_GATE_32, interrupt_redirect_for[i]);
    }

    if (legacy_pic_mode) {
        // Hardware IRQs for legacy PIC mode
        set_direct_interrupt((OFFSET_1 + 1), INT_GATE_32, irq_1);
    } else {
        // Hardware IRQs for the IOAPIC
        set_direct_interrupt((IRQ_BASE + 1), INT_GATE_32, irq_1);
    }

    // Set up Software Interrupts
    set_direct_interrupt(50, INT_GATE_32, call_int);
    set_direct_interrupt(51, INT_GATE_32, test_int);
    set_direct_interrupt(0xa0, INT_GATE_32,
                         (void (*)(interrupt_frame*))apic_int);
    set_direct_interrupt(0xa1, INT_GATE_32,
                         (void (*)(interrupt_frame*))yield_int);

    // Spurious Interrupts (0xf8 to 0xff)
    for (uint8_t i = 0xf8; i >= 0xf8; i++) {
        set_direct_interrupt(i, INT_GATE_32, spurious_int);
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
void set_direct_interrupt(uint8_t interrupt, uint8_t gate_type,
                          void (*handler)(interrupt_frame* frame)) {
    x86_tables::set_gate(idt_table[interrupt], (void*)handler,
                         x86_tables::code_selector, 0b11, gate_type, true);
}

/**
 * @brief Sets an interrupt handler in the interrupt table, and enables
 *        it's corresponding hardware interrupt on the PIC if applicable.
 *
 * @param interrupt     Number of target interrupt
 * @param gate_type     Gate type for the interrupt to be set as
 * @param handler       New interrupt handler
 */
void set_direct_interrupt(uint8_t interrupt, uint8_t gate_type,
                          void (*handler)(interrupt_frame*  frame,
                                          unsigned long int error_code)) {
    x86_tables::set_gate(idt_table[interrupt], (void*)handler,
                         x86_tables::code_selector, 0b11, gate_type, true);
}

/**
 * @brief Clears the corresponding handler for a certain interrupt
 *
 * @param interrupt     Number of target interrupt
 */
void clear_interrupt(uint8_t interrupt) {
    x86_tables::set_gate(idt_table[interrupt],
                         (void*)interrupt_redirect_for[interrupt],
                         x86_tables::code_selector, 0b11, 0, false);
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
} // namespace interrupts

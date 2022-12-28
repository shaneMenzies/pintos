#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "interrupt_redirect.h"
#include "io/io.h"
#include "libk/asm.h"
#include "libk/callable.h"
#include "threading/apic.h"

#include <stddef.h>
#include <stdint.h>

namespace acpi {
struct madt_table;
}
namespace x86_tables {
struct idt_gate;
}

namespace interrupts {

enum gate_type : uint8_t {
    INT_GATE_64 = 0x0E,
    INT_GATE_32 = 0x0E,
    INT_GATE_16 = 0x06,

    TRAP_GATE_64 = 0xf,
    TRAP_GATE_32 = 0xf,
    TRAP_GATE_16 = 0x7,

    TASK_GATE = 0x05,
};

extern x86_tables::idt_gate idt_table[256];
extern const size_t         IDT_SIZE;
extern bool                 legacy_pic_mode;
extern const uint8_t        IRQ_BASE;

void idt_init(acpi::madt_table* madt);
void interrupts_init(acpi::madt_table* madt);

void set_direct_interrupt(uint8_t interrupt, uint8_t gate_type,
                          void (*handler)(interrupt_frame* frame));
void set_direct_interrupt(uint8_t interrupt, uint8_t gate_type,
                          void (*handler)(interrupt_frame* frame,
                                          uint64_t         error_code));
void clear_interrupt(uint8_t interrupt);

/**
 * @brief Sends the End Of Interrupt signal to the APIC
 *
 */
inline void APIC_EOI() { current_apic::get_register(0xb0) = 0; };

/**
 * @brief Sends the End Of Interrupt signal to the appropiate PIC for the
 *        given IRQ trigger
 *
 * @param IRQ   Hardware interrupt to end (0-15 IRQ = 32-47 INT)
 */
inline void legacy_PIC_EOI(uint8_t IRQ) {
    if (IRQ >= 8) out_byte(PIC_2_CMD, CODE_EOI);
    out_byte(PIC_1_CMD, CODE_EOI);
};

/**
 * @brief Sends the End Of Interrupt signal to the appropriate handler
 *
 * @param interrupt   Interrupt number
 */
inline void send_EOI(uint8_t interrupt = 0) {
    if (legacy_pic_mode && (interrupt > OFFSET_1) && (interrupt < OFFSET_END)) {
        legacy_PIC_EOI((uint8_t)(interrupt - OFFSET_1));
    } else if (!legacy_pic_mode) {
        APIC_EOI();
    }
};

void PIC_set_mask(uint8_t IRQ);
void PIC_clear_mask(uint8_t IRQ);

// Exception Handlers
__attribute__((interrupt)) void div_zero(interrupt_frame* frame);
__attribute__((interrupt)) void debug(interrupt_frame* frame);
__attribute__((interrupt)) void nmi(interrupt_frame* frame);
__attribute__((interrupt)) void breakpoint_handler(interrupt_frame* frame);
__attribute__((interrupt)) void overflow(interrupt_frame* frame);
__attribute__((interrupt)) void bound_range(interrupt_frame* frame);
__attribute__((interrupt)) void invalid_opcode(interrupt_frame* frame);
__attribute__((interrupt)) void device_naval(interrupt_frame* frame);
__attribute__((interrupt)) void flt_x87(interrupt_frame* frame);
__attribute__((interrupt)) void unaligned_mem(interrupt_frame* frame);
__attribute__((interrupt)) void double_fault(interrupt_frame*  frame,
                                             unsigned long int error_code);
__attribute__((interrupt)) void gpf_handler(interrupt_frame*  frame,
                                            unsigned long int error_code);
__attribute__((interrupt)) void segment_fault(interrupt_frame*  frame,
                                              unsigned long int error_code);
__attribute__((interrupt)) void page_fault(interrupt_frame*  frame,
                                           unsigned long int error_code);

// Hardware Interrupts
__attribute__((interrupt)) void irq_0(interrupt_frame* frame);
__attribute__((interrupt)) void irq_1(interrupt_frame* frame);
__attribute__((interrupt)) void irq_2(interrupt_frame* frame);
__attribute__((interrupt)) void irq_3(interrupt_frame* frame);
__attribute__((interrupt)) void irq_4(interrupt_frame* frame);
__attribute__((interrupt)) void irq_5(interrupt_frame* frame);
__attribute__((interrupt)) void irq_6(interrupt_frame* frame);
__attribute__((interrupt)) void irq_7(interrupt_frame* frame);
__attribute__((interrupt)) void irq_8(interrupt_frame* frame);
__attribute__((interrupt)) void irq_9(interrupt_frame* frame);
__attribute__((interrupt)) void irq_10(interrupt_frame* frame);
__attribute__((interrupt)) void irq_11(interrupt_frame* frame);
__attribute__((interrupt)) void irq_12(interrupt_frame* frame);
__attribute__((interrupt)) void irq_13(interrupt_frame* frame);
__attribute__((interrupt)) void irq_14(interrupt_frame* frame);
__attribute__((interrupt)) void irq_15(interrupt_frame* frame);

// Software Interrupts
__attribute__((interrupt)) void test_int(interrupt_frame* frame);
__attribute__((interrupt)) void spurious_int(interrupt_frame* frame);
__attribute__((interrupt)) void call_int(interrupt_frame* frame);

__attribute__((interrupt)) void hpet_periodic_int(interrupt_frame* frame);
__attribute__((interrupt)) void hpet_oneshot_int(interrupt_frame* frame);

__attribute__((naked)) void apic_int();
__attribute__((naked)) void yield_int();

[[gnu::always_inline]] inline void
    prepare_call_int(std_k::callable<void>* target) {
    push_64((uint64_t)target);
    asm volatile("int 50");
}
} // namespace interrupts

#endif

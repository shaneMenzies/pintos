#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>
#include <stddef.h>

#include "x86_tables.h"
#include "io.h"
#include "terminal.h"
#include "keyboard.h"
#include "timer.h"

extern "C" {
extern void disable_interrupts();

extern void enable_interrupts();

extern void set_idt(void* idt_base, uint16_t idt_length);
}

struct interrupt_frame {
    void* return_instruction;
    uint32_t caller_segment;
    uint32_t eflags;
};

namespace interrupts {

    extern x86_tables::idt_gate idt_table[256];
    extern const size_t IDT_SIZE;

    void idt_init();
    void interrupts_init();

    void set_interrupt(uint8_t interrupt, uint8_t gate_type, void (*handler)(interrupt_frame* frame));
    void set_interrupt(uint8_t interrupt, uint8_t gate_type, void (*handler)(interrupt_frame* frame, unsigned int error_code));
    void clear_interrupt(uint8_t interrupt);

    void PIC_EOI(uint8_t IRQ);

    void PIC_set_mask(uint8_t IRQ);
    void PIC_clear_mask(uint8_t IRQ);

    // Exception Handlers
    __attribute__((interrupt)) void div_zero(interrupt_frame* frame);
    __attribute__((interrupt)) void debug(interrupt_frame* frame);
    __attribute__((interrupt)) void nmi(interrupt_frame* frame);
    __attribute__((interrupt)) void breakpoint(interrupt_frame* frame);
    __attribute__((interrupt)) void overflow(interrupt_frame* frame);
    __attribute__((interrupt)) void bound_range(interrupt_frame* frame);
    __attribute__((interrupt)) void invalid_opcode(interrupt_frame* frame);
    __attribute__((interrupt)) void device_naval(interrupt_frame* frame);
    __attribute__((interrupt)) void flt_x87(interrupt_frame* frame);
    __attribute__((interrupt)) void unaligned_mem(interrupt_frame* frame);
    __attribute__((interrupt)) void double_fault(interrupt_frame* frame, unsigned int error_code);
    __attribute__((interrupt)) void gpf_handler(interrupt_frame* frame, unsigned int error_code);
    __attribute__((interrupt)) void segment_fault(interrupt_frame* frame, unsigned int error_code);
    __attribute__((interrupt)) void page_fault(interrupt_frame* frame, unsigned int error_code);
    

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

}

#endif
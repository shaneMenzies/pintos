#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>
#include <stddef.h>

#include "x86_tables.h"
#include "io.h"
#include "terminal.h"
#include "kb_codes.h"

extern "C" {
extern void disable_interrupts();

extern void enable_interrupts();

extern void set_idt(void* idt_base, uint16_t idt_length);
}

enum : uint8_t{
    PIC_1 = 0x20,
    PIC_1_CMD = (PIC_1),
    PIC_1_DATA = (PIC_1 + 1),

    PIC_2 = 0xa0,
    PIC_2_CMD = (PIC_2),
    PIC_2_DATA = (PIC_2 + 1),

    OFFSET_1 = 0x20,
    OFFSET_2 = (OFFSET_1 + 8),
    OFFSET_END = (OFFSET_2 + 8),

    CODE_INIT = 0x11,
    CODE_HAS_SLAVE = 0x4,
    CODE_IS_SLAVE = 0x2,

    CODE_8086 = 0x1,

    CODE_EOI = 0x20,

    KB_DATA = 0x60,
    KB_CMD = 0x64,

    INT_GATE_32 = 0x0E,
    INT_GATE_16 = 0x06,

    TRAP_GATE_32 = 0xf,
    TRAP_GATE_16 = 0x7,

    TASK_GATE = 0x05,
};

struct interrupt_frame {
    void* return_instruction;
    uint32_t caller_segment;
    uint32_t eflags;
};

namespace interrupts {

    extern x86_tables::idt_gate idt_table[256];
    extern const size_t IDT_SIZE;

    void idt_init();

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
    __attribute__((interrupt)) void keyboard_handler(interrupt_frame* frame);

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
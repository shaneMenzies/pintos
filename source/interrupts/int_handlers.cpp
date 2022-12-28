/**
 * @file int_handlers.cpp
 * @author Shane Menzies
 * @brief Interrupt Handler Definitions
 * @date 03/16/21
 *
 *
 */

#include "interrupt_redirect.h"
#include "interrupts.h"
#include "io/io.h"
#include "io/keyboard.h"
#include "libk/asm.h"
#include "libk/callable.h"
#include "system/error.h"
#include "terminal/terminal.h"
#include "threading/threading.h"
#include "time/timer.h"

namespace interrupts {

/* #region Exceptions */

__attribute__((interrupt)) void div_zero(interrupt_frame* frame) {

    (void)frame;
    io_write_s("div_zero Interrupt Called", COM_1);

    raise_error(004, const_cast<char*>("div_zero"));

    // disable_interrupts();
    // while(1) {
    //     int error = error_code_addr->code;
    //     if (error == 0)
    //         break;
    // }
    // enable_interrupts();
    send_EOI();
}

__attribute__((interrupt)) void debug(interrupt_frame* frame) {

    (void)frame;
    io_write_s("Debug Interrupt Called", COM_1);

    // TODO: Print the values in the debug registers
    send_EOI();
}

__attribute__((interrupt)) void nmi(interrupt_frame* frame) {

    (void)frame;
    io_write_s("Non-Maskable Interrupt Called", COM_1);

    disable_interrupts();
    while (1) {}
    enable_interrupts();

    // TODO: Add info on where the error occurred
    send_EOI();
}

__attribute__((interrupt)) void breakpoint_handler(interrupt_frame* frame) {

    (void)frame;
    io_write_s("Breakpoint Interrupt Called", COM_1);

    volatile bool released = false;

    while (1) {
        if (!released) { asm volatile("nop"); }
    }
    send_EOI();
}

__attribute__((interrupt)) void overflow(interrupt_frame* frame) {

    (void)frame;
    io_write_s("Overflow Interrupt Called", COM_1);

    raise_error(006, const_cast<char*>("overflow"));
    send_EOI();
}

__attribute__((interrupt)) void bound_range(interrupt_frame* frame) {

    (void)frame;
    io_write_s("bound_range Interrupt Called", COM_1);

    raise_error(7, const_cast<char*>("bound_range"));
    send_EOI();
}

__attribute__((interrupt)) void invalid_opcode(interrupt_frame* frame) {

    (void)frame;
    io_write_s("Invalid Opcode Interrupt Called", COM_1);

    raise_error(8, const_cast<char*>("invalid_opcode"));
    send_EOI();
}

__attribute__((interrupt)) void device_naval(interrupt_frame* frame) {

    (void)frame;
    io_write_s("device_not_available Interrupt Called", COM_1);

    raise_error(9, const_cast<char*>("device_naval"));
    send_EOI();
}

__attribute__((interrupt)) void flt_x87(interrupt_frame* frame) {

    (void)frame;
    io_write_s("flt_x87 Interrupt Called", COM_1);

    raise_error(13, const_cast<char*>("flt_x87"));
    send_EOI();
}

__attribute__((interrupt)) void unaligned_mem(interrupt_frame* frame) {

    (void)frame;
    io_write_s("unaligned_mem Interrupt Called", COM_1);

    raise_error(14, const_cast<char*>("unaligned_mem"));
    send_EOI();
}

__attribute__((interrupt)) void gpf_handler(interrupt_frame*  frame,
                                            unsigned long int error_code) {

    (void)frame;
    io_write_s("gpf_handler Interrupt Called", COM_1);

    raise_error(003, const_cast<char*>("gpf_handler"));
    active_terminal->tprintf(const_cast<char*>("Error Code: %x\n"), error_code);
    active_terminal->update();

    // disable_interrupts();
    // while(1) {
    //     int error = error_code_addr->code;
    //     if (error == 0)
    //         break;
    // }
    // enable_interrupts();
    send_EOI();
}

__attribute__((interrupt)) void double_fault(interrupt_frame*  frame,
                                             unsigned long int error_code) {

    (void)frame;
    (void)error_code;
    io_write_s("double_fault Interrupt Called", COM_1);

    raise_error(10, const_cast<char*>("double_fault"));

    disable_interrupts();
    while (1) {}
    enable_interrupts();
    send_EOI();
}

__attribute__((interrupt)) void segment_fault(interrupt_frame*  frame,
                                              unsigned long int error_code) {

    (void)frame;
    io_write_s("segment_fault Interrupt Called", COM_1);

    raise_error(11, const_cast<char*>("segment_fault"));
    active_terminal->tprintf(const_cast<char*>("Error Code: %x\n"), error_code);
    active_terminal->update();

    disable_interrupts();
    while (1) {}
    enable_interrupts();
    send_EOI();
}

__attribute__((interrupt)) void page_fault(interrupt_frame*  frame,
                                           unsigned long int error_code) {

    (void)frame;
    io_write_s("page_fault Interrupt Called", COM_1);

    raise_error(003, const_cast<char*>("page_fault"));
    active_terminal->tprintf(const_cast<char*>("Error Code: %x\n"), error_code);
    active_terminal->update();

    disable_interrupts();
    while (1) {}
    enable_interrupts();

    send_EOI();
}

/* #endregion */

/* #region Hardware Interrupts */

__attribute__((interrupt)) void irq_0(interrupt_frame* frame) {

    (void)frame;

    send_EOI(IRQ_BASE + 0);
}

__attribute__((interrupt)) void irq_1(interrupt_frame* frame) {

    (void)frame;

    unsigned char scan_code = in_byte(KB_DATA);
    io_write_c(scan_code, IO_ports::COM_1);

    if (scan_code > 0x7f) {
        keyboard::key_break(scan_code);
    } else {
        keyboard::key_make(scan_code);
    }

    send_EOI(IRQ_BASE + 1);
}

__attribute__((interrupt)) void irq_2(interrupt_frame* frame) {

    (void)frame;

    io_write_s("IRQ 2 Called", COM_1);
    active_terminal->write_s(const_cast<char*>("\nIRQ 2 Called\n"));
    active_terminal->update();
    send_EOI(IRQ_BASE + 2);
}

__attribute__((interrupt)) void irq_3(interrupt_frame* frame) {

    (void)frame;

    io_write_s("IRQ 3 Called", COM_1);
    active_terminal->write_s(const_cast<char*>("\nIRQ 3 Called\n"));
    active_terminal->update();
    send_EOI(IRQ_BASE + 3);
}

__attribute__((interrupt)) void irq_4(interrupt_frame* frame) {

    (void)frame;

    // COM_1 INTERRUPT
    serial::serial_handlers[serial::serial_handler_identities::COM_1]
        .device_interrupt();
    send_EOI(IRQ_BASE + 4);
}

__attribute__((interrupt)) void irq_5(interrupt_frame* frame) {

    (void)frame;

    // COM_2 INTERRUPT
    serial::serial_handlers[serial::serial_handler_identities::COM_2]
        .device_interrupt();
    send_EOI(IRQ_BASE + 5);
}

__attribute__((interrupt)) void irq_6(interrupt_frame* frame) {

    (void)frame;

    io_write_s("IRQ 6 Called", COM_1);
    active_terminal->write_s(const_cast<char*>("\nIRQ 6 Called\n"));
    active_terminal->update();
    send_EOI(IRQ_BASE + 6);
}

__attribute__((interrupt)) void irq_7(interrupt_frame* frame) {

    (void)frame;

    io_write_s("IRQ 7 Called", COM_1);
    active_terminal->write_s(const_cast<char*>("\nIRQ 7 Called\n"));
    active_terminal->update();
    send_EOI(IRQ_BASE + 7);
}

__attribute__((interrupt)) void irq_8(interrupt_frame* frame) {

    (void)frame;

    io_write_s("IRQ 8 Called", COM_1);
    active_terminal->write_s(const_cast<char*>("\nIRQ 8 Called\n"));
    active_terminal->update();
    send_EOI(IRQ_BASE + 8);
}

__attribute__((interrupt)) void irq_9(interrupt_frame* frame) {

    (void)frame;

    io_write_s("IRQ 9 Called", COM_1);
    active_terminal->write_s(const_cast<char*>("\nIRQ 9 Called\n"));
    active_terminal->update();
    send_EOI(IRQ_BASE + 9);
}

__attribute__((interrupt)) void irq_10(interrupt_frame* frame) {

    (void)frame;

    io_write_s("IRQ 10 Called", COM_1);
    active_terminal->write_s(const_cast<char*>("\nIRQ 10 Called\n"));
    active_terminal->update();
    send_EOI(IRQ_BASE + 10);
}

__attribute__((interrupt)) void irq_11(interrupt_frame* frame) {

    (void)frame;

    io_write_s("IRQ 11 Called", COM_1);
    active_terminal->write_s(const_cast<char*>("\nIRQ 11 Called\n"));
    active_terminal->update();
    send_EOI(IRQ_BASE + 11);
}

__attribute__((interrupt)) void irq_12(interrupt_frame* frame) {

    (void)frame;

    io_write_s("IRQ 12 Called", COM_1);
    active_terminal->write_s(const_cast<char*>("\nIRQ 12 Called\n"));
    active_terminal->update();
    send_EOI(IRQ_BASE + 12);
}

__attribute__((interrupt)) void irq_13(interrupt_frame* frame) {

    (void)frame;

    io_write_s("IRQ 13 Called", COM_1);
    active_terminal->write_s(const_cast<char*>("\nIRQ 13 Called\n"));
    active_terminal->update();
    send_EOI(IRQ_BASE + 13);
}

__attribute__((interrupt)) void irq_14(interrupt_frame* frame) {

    (void)frame;

    io_write_s("IRQ 14 Called", COM_1);
    active_terminal->write_s(const_cast<char*>("\nIRQ 14 Called\n"));
    active_terminal->update();
    send_EOI(IRQ_BASE + 14);
}

__attribute__((interrupt)) void irq_15(interrupt_frame* frame) {

    (void)frame;

    io_write_s("IRQ 15 Called", COM_1);
    active_terminal->write_s(const_cast<char*>("\nIRQ 15 Called\n"));
    active_terminal->update();
    send_EOI(IRQ_BASE + 15);
}

/* #endregion */

/* #region Software Interrupts */

__attribute__((interrupt)) void test_int(interrupt_frame* frame) {

    (void)frame;

    io_write_s("Interrupt 51 Called", COM_1);
    active_terminal->write_s(const_cast<char*>("\nInterrupt success!\n"));
    active_terminal->update();
    send_EOI();
}

__attribute__((interrupt)) void spurious_int(interrupt_frame* frame) {

    (void)frame;
    send_EOI();
}

__attribute__((interrupt)) void call_int(interrupt_frame* frame) {
    // Calls address from top of caller stack
    frame->return_stack_pointer += sizeof(std_k::callable<void>*);
    std_k::callable<void>* target
        = (std_k::callable<void>*)(*((uint64_t*)frame->return_stack_pointer));
    target->call();
}

/* #endregion*/

extern "C" {
void real_apic_int(general_regs_state* task_regs, interrupt_frame* task_frame) {
    // Find this core's scheduler
    threading::thread_scheduler* scheduler = current_thread()->scheduler;

    // Update scheduling
    scheduler->scheduling_function.set_args(scheduler, task_regs, task_frame);

    // Increment this thread's timer
    scheduler->local_timer->run();

    send_EOI();
}
}

__attribute__((naked)) void apic_int() {
    asm volatile("cli\n\t");
    PUSH_GENERAL_REGS();
    asm volatile("lea (%rsp), %rdi \n\t\
             lea 0x78(%rsp), %rsi \n\t\
             call real_apic_int \n\t");
    POP_GENERAL_REGS();
    asm volatile("sti\n\t");
    asm volatile("iretq \n\t");
}

extern "C" {
void real_yield_int(general_regs_state* task_regs,
                    interrupt_frame*    task_frame) {
    // Find this core's scheduler
    threading::thread_scheduler* scheduler = current_thread()->scheduler;

    scheduler->yield_current(task_regs, task_frame);

    send_EOI();
}
}

__attribute__((naked)) void yield_int() {
    asm volatile("cli\n\t");
    PUSH_GENERAL_REGS();
    asm volatile("lea (%rsp), %rdi \n\t\
             lea 0x78(%rsp), %rsi \n\t\
             call real_yield_int \n\t");
    POP_GENERAL_REGS();
    asm volatile("sti\n\t");
    asm volatile("iretq \n\t");
}
} // namespace interrupts

/**
 * @file int_handlers.cpp
 * @author Shane Menzies
 * @brief Interrupt Handler Definitions
 * @date 03/16/21
 * 
 * 
 */

#include "interrupts.h"
#include "timer.h"

namespace interrupts {

    /* #region Exceptions */

    __attribute__((interrupt)) void div_zero(interrupt_frame* frame) {

        (void) frame;
        serial::write_s("div_zero Interrupt Called", COM_1);

        raise_error(004, const_cast<char*>("div_zero"));
        
        //disable_interrupts();
        //while(1) {
        //    int error = error_code_addr->code;
        //    if (error == 0)
        //        break;
        //}
        //enable_interrupts();
        send_EOI();
    }

    __attribute__((interrupt)) void debug(interrupt_frame* frame) {

        (void) frame;
        serial::write_s("Debug Interrupt Called", COM_1);
        
        // TODO: Print the values in the debug registers
        send_EOI();
    }

    __attribute__((interrupt)) void nmi(interrupt_frame* frame) {

        (void) frame;
        serial::write_s("Non-Maskable Interrupt Called", COM_1);
        
        disable_interrupts();
        while(1) {
        }
        enable_interrupts();

        // TODO: Add info on where the error occurred
        send_EOI();
    }

    __attribute__((interrupt)) void breakpoint(interrupt_frame* frame) {

        (void) frame;
        serial::write_s("Breakpoint Interrupt Called", COM_1);
        
        while (1) {}
        send_EOI();
    }

    __attribute__((interrupt)) void overflow(interrupt_frame* frame) {

        (void) frame;
        serial::write_s("Overflow Interrupt Called", COM_1);
        
        raise_error(006, const_cast<char*>("overflow"));
        send_EOI();
    }

    __attribute__((interrupt)) void bound_range(interrupt_frame* frame) {

        (void) frame;
        serial::write_s("bound_range Interrupt Called", COM_1);
        
        raise_error(7, const_cast<char*>("bound_range"));
        send_EOI();
    }

    __attribute__((interrupt)) void invalid_opcode(interrupt_frame* frame) {

        (void) frame;
        serial::write_s("Invalid Opcode Interrupt Called", COM_1);
        
        raise_error(8, const_cast<char*>("invalid_opcode"));
        send_EOI();
    }

    __attribute__((interrupt)) void device_naval(interrupt_frame* frame) {

        (void) frame;
        serial::write_s("device_not_available Interrupt Called", COM_1);
        
        raise_error(9, const_cast<char*>("device_naval"));
        send_EOI();
    }

    __attribute__((interrupt)) void flt_x87(interrupt_frame* frame) {

        (void) frame;
        serial::write_s("flt_x87 Interrupt Called", COM_1);
        
        raise_error(13, const_cast<char*>("flt_x87"));
        send_EOI();
    }

    __attribute__((interrupt)) void unaligned_mem(interrupt_frame* frame) {

        (void) frame;
        serial::write_s("unaligned_mem Interrupt Called", COM_1);
        
        raise_error(14, const_cast<char*>("unaligned_mem"));
        send_EOI();
    }

    __attribute__((interrupt)) void gpf_handler(interrupt_frame* frame, unsigned long int error_code) {

        (void) frame;
        serial::write_s("gpf_handler Interrupt Called", COM_1);

        raise_error(003, const_cast<char*>("gpf_handler"));
        active_terminal->tprintf(const_cast<char*>("Error Code: %x\n"), error_code);
        active_terminal->update();
        
        //disable_interrupts();
        //while(1) {
        //    int error = error_code_addr->code;
        //    if (error == 0)
        //        break;
        //}
        //enable_interrupts();
        send_EOI();
    }

    __attribute__((interrupt)) void double_fault(interrupt_frame* frame, unsigned long int error_code) {

        (void) frame;
        (void) error_code;
        serial::write_s("double_fault Interrupt Called", COM_1);

        raise_error(10, const_cast<char*>("double_fault"));
        
        disable_interrupts();
        while(1) {
        }
        enable_interrupts();
        send_EOI();
    }

    __attribute__((interrupt)) void segment_fault(interrupt_frame* frame, unsigned long int error_code) {

        (void) frame;
        serial::write_s("segment_fault Interrupt Called", COM_1);

        raise_error(11, const_cast<char*>("segment_fault"));
        active_terminal->tprintf(const_cast<char*>("Error Code: %x\n"), error_code);
        active_terminal->update();
        
        disable_interrupts();
        while(1) {
        }
        enable_interrupts();
        send_EOI();
    }

    __attribute__((interrupt)) void page_fault(interrupt_frame* frame, unsigned long int error_code) {

        (void) frame;
        serial::write_s("page_fault Interrupt Called", COM_1);

        raise_error(003, const_cast<char*>("page_fault"));
        active_terminal->tprintf(const_cast<char*>("Error Code: %x\n"), error_code);
        active_terminal->update();
        
        disable_interrupts();
        while(1) {
        }
        enable_interrupts();

        send_EOI();
    }

    /* #endregion */

    /* #region Hardware Interrupts */

    __attribute__((interrupt)) void irq_0(interrupt_frame* frame) {

        (void) frame;

        timer::sys_int_timer->run_tasks();

        send_EOI(IRQ_BASE + 0);
    }

    __attribute__((interrupt)) void irq_1(interrupt_frame* frame) {

        (void) frame;

        unsigned char scan_code = in_byte(KB_DATA);

        if (scan_code > 0x7f) {
            keyboard::key_break(scan_code);
        } else {
            keyboard::key_make(scan_code);
        }

        send_EOI(IRQ_BASE + 1);
    }

    __attribute__((interrupt)) void irq_2(interrupt_frame* frame) {

        (void) frame;

        serial::write_s("IRQ 2 Called", COM_1);
        active_terminal->write_s(const_cast<char*>("\nIRQ 2 Called\n"));
        active_terminal->update();
        send_EOI(IRQ_BASE + 2);
    }

    __attribute__((interrupt)) void irq_3(interrupt_frame* frame) {

        (void) frame;

        serial::write_s("IRQ 3 Called", COM_1);
        active_terminal->write_s(const_cast<char*>("\nIRQ 3 Called\n"));
        active_terminal->update();
        send_EOI(IRQ_BASE + 3);
    }

    __attribute__((interrupt)) void irq_4(interrupt_frame* frame) {

        (void) frame;

        serial::write_s("IRQ 4 Called", COM_1);
        active_terminal->write_s(const_cast<char*>("\nIRQ 4 Called\n"));
        active_terminal->update();
        send_EOI(IRQ_BASE + 4);
    }

    __attribute__((interrupt)) void irq_5(interrupt_frame* frame) {

        (void) frame;

        serial::write_s("IRQ 5 Called", COM_1);
        active_terminal->write_s(const_cast<char*>("\nIRQ 5 Called\n"));
        active_terminal->update();
        send_EOI(IRQ_BASE + 5);
    }

    __attribute__((interrupt)) void irq_6(interrupt_frame* frame) {

        (void) frame;

        serial::write_s("IRQ 6 Called", COM_1);
        active_terminal->write_s(const_cast<char*>("\nIRQ 6 Called\n"));
        active_terminal->update();
        send_EOI(IRQ_BASE + 6);
    }

    __attribute__((interrupt)) void irq_7(interrupt_frame* frame) {

        (void) frame;

        serial::write_s("IRQ 7 Called", COM_1);
        active_terminal->write_s(const_cast<char*>("\nIRQ 7 Called\n"));
        active_terminal->update();
        send_EOI(IRQ_BASE + 7);
    }

    __attribute__((interrupt)) void irq_8(interrupt_frame* frame) {

        (void) frame;

        serial::write_s("IRQ 8 Called", COM_1);
        active_terminal->write_s(const_cast<char*>("\nIRQ 8 Called\n"));
        active_terminal->update();
        send_EOI(IRQ_BASE + 8);
    }

    __attribute__((interrupt)) void irq_9(interrupt_frame* frame) {

        (void) frame;

        serial::write_s("IRQ 9 Called", COM_1);
        active_terminal->write_s(const_cast<char*>("\nIRQ 9 Called\n"));
        active_terminal->update();
        send_EOI(IRQ_BASE + 9);
    }

    __attribute__((interrupt)) void irq_10(interrupt_frame* frame) {

        (void) frame;

        serial::write_s("IRQ 10 Called", COM_1);
        active_terminal->write_s(const_cast<char*>("\nIRQ 10 Called\n"));
        active_terminal->update();
        send_EOI(IRQ_BASE + 10);
    }

    __attribute__((interrupt)) void irq_11(interrupt_frame* frame) {

        (void) frame;

        serial::write_s("IRQ 11 Called", COM_1);
        active_terminal->write_s(const_cast<char*>("\nIRQ 11 Called\n"));
        active_terminal->update();
        send_EOI(IRQ_BASE + 11);
    }

    __attribute__((interrupt)) void irq_12(interrupt_frame* frame) {

        (void) frame;

        serial::write_s("IRQ 12 Called", COM_1);
        active_terminal->write_s(const_cast<char*>("\nIRQ 12 Called\n"));
        active_terminal->update();
        send_EOI(IRQ_BASE + 12);
    }

    __attribute__((interrupt)) void irq_13(interrupt_frame* frame) {

        (void) frame;

        serial::write_s("IRQ 13 Called", COM_1);
        active_terminal->write_s(const_cast<char*>("\nIRQ 13 Called\n"));
        active_terminal->update();
        send_EOI(IRQ_BASE + 13);
    }

    __attribute__((interrupt)) void irq_14(interrupt_frame* frame) {

        (void) frame;

        serial::write_s("IRQ 14 Called", COM_1);
        active_terminal->write_s(const_cast<char*>("\nIRQ 14 Called\n"));
        active_terminal->update();
        send_EOI(IRQ_BASE + 14);
    }

    __attribute__((interrupt)) void irq_15(interrupt_frame* frame) {

        (void) frame;

        serial::write_s("IRQ 15 Called", COM_1);
        active_terminal->write_s(const_cast<char*>("\nIRQ 15 Called\n"));
        active_terminal->update();
        send_EOI(IRQ_BASE + 15);
    }

    /* #endregion */

    /* #region Software Interrupts */

    __attribute__((interrupt)) void test_int(interrupt_frame* frame) {

        (void) frame;

        serial::write_s("Interrupt 51 Called", COM_1);
        active_terminal->write_s(const_cast<char*>("\nInterrupt success!\n"));
        active_terminal->update();
        send_EOI();
    }

    __attribute__((interrupt)) void spurious_int(interrupt_frame* frame) {

        (void) frame;
        send_EOI();
    }

    /* #endregion*/

    __attribute__((interrupt)) void hpet_periodic_int(interrupt_frame* frame) {

        (void) frame;

        timer::sys_int_timer->run_tasks();

        send_EOI(OFFSET_1 + 0);
    }

    __attribute__((interrupt)) void hpet_oneshot_int(interrupt_frame* frame) {

        (void) frame;

        // Reset HPET counter to 0
        *((volatile uint64_t*)((uintptr_t)timer::hpet_timer->address + 0xf0)) = 0;

        timer::sys_int_timer->run_tasks();
        send_EOI(OFFSET_1 + 0);
    }
}
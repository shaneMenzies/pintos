/**
 * @file int_handlers.cpp
 * @author Shane Menzies
 * @brief Interrupt Handler Definitions
 * @date 03/16/21
 * 
 * 
 */

#include "interrupts.h"

namespace interrupts {

    /* #region Exceptions */

    __attribute__((interrupt)) void div_zero(interrupt_frame* frame) {

        (void) frame;

        raise_error(004, const_cast<char*>("div_zero"));
        
        disable_interrupts();
        while(1) {
            int error = error_code_addr->code;
            if (error == 0)
                break;
        }
        enable_interrupts();
    }

    __attribute__((interrupt)) void debug(interrupt_frame* frame) {

        (void) frame;
        
        // TODO: Print the values in the debug registers
    }

    __attribute__((interrupt)) void nmi(interrupt_frame* frame) {

        (void) frame;
        
        disable_interrupts();
        while(1) {
            int error = error_code_addr->code;
            if (error == 0)
                break;
        }
        enable_interrupts();

        // TODO: Add info on where the error occurred
    }

    __attribute__((interrupt)) void breakpoint(interrupt_frame* frame) {

        (void) frame;
        
        while (1) {}
    }

    __attribute__((interrupt)) void overflow(interrupt_frame* frame) {

        (void) frame;
        
        raise_error(006, const_cast<char*>("overflow"));
    }

    __attribute__((interrupt)) void bound_range(interrupt_frame* frame) {

        (void) frame;
        
        raise_error(7, const_cast<char*>("bound_range"));
    }

    __attribute__((interrupt)) void invalid_opcode(interrupt_frame* frame) {

        (void) frame;
        
        raise_error(8, const_cast<char*>("invalid_opcode"));
    }

    __attribute__((interrupt)) void device_naval(interrupt_frame* frame) {

        (void) frame;
        
        raise_error(9, const_cast<char*>("device_naval"));
    }

    __attribute__((interrupt)) void flt_x87(interrupt_frame* frame) {

        (void) frame;
        
        raise_error(13, const_cast<char*>("flt_x87"));
    }

    __attribute__((interrupt)) void unaligned_mem(interrupt_frame* frame) {

        (void) frame;
        
        raise_error(14, const_cast<char*>("unaligned_mem"));
    }

    __attribute__((interrupt)) void gpf_handler(interrupt_frame* frame, unsigned int error_code) {

        (void) frame;

        raise_error(003, const_cast<char*>("gpf_handler"));
        active_terminal->tprintf(const_cast<char*>("Error Code: %x\n"), error_code);
        active_terminal->update();
        
        disable_interrupts();
        while(1) {
            int error = error_code_addr->code;
            if (error == 0)
                break;
        }
        enable_interrupts();
    }

    __attribute__((interrupt)) void double_fault(interrupt_frame* frame, unsigned int error_code) {

        (void) frame;
        (void) error_code;

        raise_error(10, const_cast<char*>("double_fault"));
        
        disable_interrupts();
        while(1) {
            int error = error_code_addr->code;
            if (error == 0)
                break;
        }
        enable_interrupts();
    }

    __attribute__((interrupt)) void segment_fault(interrupt_frame* frame, unsigned int error_code) {

        (void) frame;

        raise_error(11, const_cast<char*>("segment_fault"));
        active_terminal->tprintf(const_cast<char*>("Error Code: %x\n"), error_code);
        active_terminal->update();
        
        disable_interrupts();
        while(1) {
            int error = error_code_addr->code;
            if (error == 0)
                break;
        }
        enable_interrupts();
    }

    __attribute__((interrupt)) void page_fault(interrupt_frame* frame, unsigned int error_code) {

        (void) frame;

        raise_error(003, const_cast<char*>("page_fault"));
        active_terminal->tprintf(const_cast<char*>("Error Code: %x\n"), error_code);
        active_terminal->update();
        
        disable_interrupts();
        while(1) {
            int error = error_code_addr->code;
            if (error == 0)
                break;
        }
        enable_interrupts();
    }

    /* #endregion */

    /* #region Hardware Interrupts */

    __attribute__((interrupt)) void irq_0(interrupt_frame* frame) {

        (void) frame;

        disable_interrupts();

        timer::sys_int_timer->run_tasks();

        PIC_EOI(0);
        enable_interrupts();
    }

    __attribute__((interrupt)) void irq_1(interrupt_frame* frame) {

        (void) frame;

        unsigned char scan_code = in_byte(KB_DATA);

        if (scan_code > 0x7f) {
            keyboard::key_break(scan_code);
        } else {
            keyboard::key_make(scan_code);
        }

        PIC_EOI(1);

    }

    __attribute__((interrupt)) void irq_2(interrupt_frame* frame) {

        (void) frame;

        active_terminal->write_s(const_cast<char*>("\nIRQ 2 Called\n"));
        active_terminal->update();
        PIC_EOI(2);
    }

    __attribute__((interrupt)) void irq_3(interrupt_frame* frame) {

        (void) frame;

        active_terminal->write_s(const_cast<char*>("\nIRQ 3 Called\n"));
        active_terminal->update();
        PIC_EOI(3);
    }

    __attribute__((interrupt)) void irq_4(interrupt_frame* frame) {

        (void) frame;

        active_terminal->write_s(const_cast<char*>("\nIRQ 4 Called\n"));
        active_terminal->update();
        PIC_EOI(4);
    }

    __attribute__((interrupt)) void irq_5(interrupt_frame* frame) {

        (void) frame;

        active_terminal->write_s(const_cast<char*>("\nIRQ 5 Called\n"));
        active_terminal->update();
        PIC_EOI(5);
    }

    __attribute__((interrupt)) void irq_6(interrupt_frame* frame) {

        (void) frame;

        active_terminal->write_s(const_cast<char*>("\nIRQ 6 Called\n"));
        active_terminal->update();
        PIC_EOI(6);
    }

    __attribute__((interrupt)) void irq_7(interrupt_frame* frame) {

        (void) frame;

        active_terminal->write_s(const_cast<char*>("\nIRQ 7 Called\n"));
        active_terminal->update();
        PIC_EOI(7);
    }

    __attribute__((interrupt)) void irq_8(interrupt_frame* frame) {

        (void) frame;

        active_terminal->write_s(const_cast<char*>("\nIRQ 8 Called\n"));
        active_terminal->update();
        PIC_EOI(8);
    }

    __attribute__((interrupt)) void irq_9(interrupt_frame* frame) {

        (void) frame;

        active_terminal->write_s(const_cast<char*>("\nIRQ 9 Called\n"));
        active_terminal->update();
        PIC_EOI(9);
    }

    __attribute__((interrupt)) void irq_10(interrupt_frame* frame) {

        (void) frame;

        active_terminal->write_s(const_cast<char*>("\nIRQ 10 Called\n"));
        active_terminal->update();
        PIC_EOI(10);
    }

    __attribute__((interrupt)) void irq_11(interrupt_frame* frame) {

        (void) frame;

        active_terminal->write_s(const_cast<char*>("\nIRQ 11 Called\n"));
        active_terminal->update();
        PIC_EOI(11);
    }

    __attribute__((interrupt)) void irq_12(interrupt_frame* frame) {

        (void) frame;

        active_terminal->write_s(const_cast<char*>("\nIRQ 12 Called\n"));
        active_terminal->update();
        PIC_EOI(12);
    }

    __attribute__((interrupt)) void irq_13(interrupt_frame* frame) {

        (void) frame;

        active_terminal->write_s(const_cast<char*>("\nIRQ 13 Called\n"));
        active_terminal->update();
        PIC_EOI(13);
    }

    __attribute__((interrupt)) void irq_14(interrupt_frame* frame) {

        (void) frame;

        active_terminal->write_s(const_cast<char*>("\nIRQ 14 Called\n"));
        active_terminal->update();
        PIC_EOI(14);
    }

    __attribute__((interrupt)) void irq_15(interrupt_frame* frame) {

        (void) frame;

        active_terminal->write_s(const_cast<char*>("\nIRQ 15 Called\n"));
        active_terminal->update();
        PIC_EOI(15);
    }

    /* #endregion */

    /* #region Software Interrupts */

    __attribute__((interrupt)) void test_int(interrupt_frame* frame) {

        (void) frame;

        active_terminal->write_s(const_cast<char*>("\nInterrupt success!\n"));
        active_terminal->update();
    }

    /* #endregion*/
}
/**
 * @file init.c
 * @author Shane Menzies
 * @brief Initializes various requirements required by the main kernel
 * @date 02/24/21
 * 
 * 
 */

#include "init.h"

extern "C" {
    extern uintptr_t kernel_start;
    extern uintptr_t kernel_end;
}

struct mb_info* mb_ptr;

/**
 * @brief Error function as result from the call of a purely
 *        virtual function
 * 
 */
void __cxa_pure_virtual() {
    raise_error(002, const_cast<char*>("__cxa_pure_virtual"));
}

extern "C" {

/**
 * @brief Early Initialization function, runs before _init
 * 
 */
void early_init() {

    mb_ptr = (struct mb_info*) return_ebx();
    return;
}

/**
 * @brief Late Initialization function, runs before kernel_main
 * 
 */
void late_init() {

    // Make sure interrupts are disabled
    disable_interrupts();

    // Initialize Memory
    memory_init(mb_ptr);

    // Allocate the Reserved areas of Memory
    // Other sections of BIOS memory already reserved by GRUB or included in kernel area
    // Allocate the area which the kernel is loaded into
    talloc((void*)&kernel_start, (size_t)( (uintptr_t)&kernel_end - (uintptr_t)&kernel_start ) );

    // Set up boot terminal
    boot_terminal = new terminal();
    active_terminal = boot_terminal;
    active_terminal->write(const_cast<char*>("PintOS Booting..."));

    // Set up the framebuffer
    framebuffer_init(mb_ptr);
    active_terminal->show();

    // Allocate error code section
    error_code_addr = (error_code_section*)malloc(sizeof(error_code_section));

    // Initialize the gdt
    gdt_init();

    // Initialize the interrupt table
    interrupts::idt_init();

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
    //interrupts::set_interrupt((OFFSET_1 + 0), INT_GATE_32, interrupts::irq_0);
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

    // Set up test interrupt
    interrupts::set_interrupt(51, INT_GATE_32, interrupts::test_int);

    // Can now enable interrupts
    enable_interrupts();
}
}
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

    // Set default interrupt handlers
    interrupts::interrupts_init();

    // Set up test interrupt
    interrupts::set_interrupt(51, INT_GATE_32, interrupts::test_int);

    // Can now enable interrupts
    enable_interrupts();
}
}
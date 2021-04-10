/**
 * @file init.cpp
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

struct mb_info main_mb_info;
bool float_support = false;

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

    // Load all the multiboot info into a known structure before 
    // it can get clobbered up by other things
    main_mb_info = *(mb_info*)return_ebx();
    return;
}

/**
 * @brief Late Initialization function, runs before kernel_main
 * 
 */
void late_init() {

    // Make sure interrupts are disabled
    disable_interrupts();

    // Initialize Floating Point support
    float_support = fpu_init();

    // Initialize Memory
    memory_init(&main_mb_info);
    talloc((void*)&kernel_start, (size_t)( (uintptr_t)&kernel_end - (uintptr_t)&kernel_start ) );

    // Create the kernel command line and master terminal
    kernel::cmd_init();
    log_terminal = new terminal();

    // Start system timer
    timer::sys_timer_init(32);

    // Set up the framebuffer
    framebuffer_init(&main_mb_info);

    // Add cursor drawing
    timer::sys_int_timer->push_task(16, draw_active_cursor);

    // Set up boot terminal
    boot_terminal = new visual_terminal();
    active_terminal = boot_terminal;
    active_terminal->write_s(const_cast<char*>("PintOS Booting..."));
    active_terminal->update();

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
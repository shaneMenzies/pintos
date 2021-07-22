/**
 * @file init.cpp
 * @author Shane Menzies
 * @brief Initializes various requirements required by the main kernel
 * @date 02/24/21
 * 
 * 
 */

#include "init.h"

/**
 * @brief Error function as result from the call of a purely
 *        virtual function
 * 
 */
void __cxa_pure_virtual() {
    raise_error(002, const_cast<char*>("__cxa_pure_virtual"));
}

extern "C" {

void _start(multiboot_boot_info* mb_info) {

    early_init();
    _init();
    late_init(mb_info);
    kernel_main();
    _fini();
}

bool float_support = false;

/**
 * @brief Early Initialization function, runs before _init
 * 
 */
void early_init() {

    // Initialize Floating Point support
    float_support = fpu_init();

    // Check control registers
    uint64_t cr0_info = 0;
    uint64_t cr4_info = 0;
    uint16_t cs_info = 0;
    uint16_t ds_info = 0;
    char string_buffer[1024];

    asm volatile (
        "movq %%cr0, %%rax \n\t\
         movq %%rax, %[cr0_info] \n\t\
         movq %%cr4, %%rax \n\t\
         movq %%rax, %[cr4_info] \n\t\
         movw %%cs, %[cs_info] \n\t\
         movw %%ds, %[ds_info] \n\t"
        : [cr0_info] "=g" (cr0_info), [cr4_info] "=g" (cr4_info), [cs_info] "=g" (cs_info), [ds_info] "=g" (ds_info)
        :
        : "rax"
    );

    char format[] = "CR0: %b \r\nCR4: %b \r\nCS: %x \r\nDS: %x\r\n";

    printf(string_buffer, format, cr0_info, cr4_info, cs_info, ds_info);

    serial::write_s(string_buffer, IO_ports::COM_1);

    if (float_support) {
        out_byte('Y', 0x3f8);
    } else {
        out_byte('N', 0x3f8);
    }

    return;
}

/**
 * @brief Late Initialization function, runs before kernel_main
 * 
 */
void late_init(multiboot_boot_info* mb_info) {

    // Initialize Memory Managment
    memory_init(mb_info);

    // Create the kernel command line and master terminal
    kernel::cmd_init();
    log_terminal = new terminal();
    
    // Start system timer
    timer::sys_timer_init(32);

    // Set up the framebuffer
    framebuffer_init(mb_info);

    // Add cursor drawing
    timer::sys_int_timer->push_task(1, draw_active_cursor);

    // Set up boot terminals
    boot_terminal = new visual_terminal();
    set_error_terminal(new visual_terminal());
    active_terminal = boot_terminal;
    active_terminal->write_s(const_cast<char*>("PintOS Booting..."));
    active_terminal->update();

    // Find the ACPI RSDP
    acpi::old_rsdp* rsdp = acpi::find_rsdp(mb_info);

    // Set default interrupt handlers
    interrupts::interrupts_init((acpi::madt_table*)acpi::get_table(rsdp, acpi::table_signature::MADT));

    // Can now enable interrupts
    enable_interrupts();

    asm volatile("int $51");

    asm volatile("int $33");

    while(1) {}

    uint64_t test_counting_number = 0;
    const char format_string[] = "Current Count: %x \r\n";
    char print_string[256];

    while(1) {
        printf(print_string, format_string, test_counting_number);
        serial::write_s(print_string, COM_1);
        test_counting_number++;
    }
}
}
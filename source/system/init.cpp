/**
 * @file init.cpp
 * @author Shane Menzies
 * @brief Initializes various requirements required by the main kernel
 * @date 02/24/21
 *
 *
 */

#include "init.h"

#include "../../include/libk/asm.h"
#include "acpi.h"
#include "display/display.h"
#include "error.h"
#include "interrupts/interrupts.h"
#include "io/io.h"
#include "kernel.h"
#include "libk/cstring.h"
#include "libk/random.h"
#include "memory/addressing.h"
#include "memory/paging.h"
#include "memory/x86_tables.h"
#include "multiboot.h"
#include "pintos_std.h"
#include "terminal/commands.h"
#include "terminal/terminal.h"
#include "threading/threading.h"
#include "time/hpet.h"
#include "time/timer.h"

#ifdef DEBUG
    #include "debug/debug_build.h"
#endif

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

    // SSE instructions will need a 16-byte aligned stack reference
    uint64_t old_rsp = get_rsp();
    if (old_rsp % 0x10) { set_rsp(old_rsp + 0x8); }

    early_init(mb_info);
    _init();
    late_init(mb_info);
    active_terminal->write_s(
        const_cast<char*>("Returned from initialization?\n"));
    _fini();
}

bool float_support = false;
bool initialized   = false;

/**
 * @brief Early Initialization function, runs before _init
 *
 */
void early_init(multiboot_boot_info* mb_info) {

    disable_interrupts();

    // Initialize Floating Point support
    float_support = fpu_init();

    // Seed rng
    std_k::seed_rand();

    // Check control registers
    uint64_t cr0_info = 0;
    uint64_t cr4_info = 0;
    uint16_t cs_info  = 0;
    uint16_t ds_info  = 0;
    char     string_buffer[1024];

    asm volatile("movq %%cr0, %%rax \n\t\
         movq %%rax, %[cr0_info] \n\t\
         movq %%cr4, %%rax \n\t\
         movq %%rax, %[cr4_info] \n\t\
         movw %%cs, %[cs_info] \n\t\
         movw %%ds, %[ds_info] \n\t"
                 : [cr0_info] "=g"(cr0_info), [cr4_info] "=g"(cr4_info),
                   [cs_info] "=g"(cs_info), [ds_info] "=g"(ds_info)
                 :
                 : "rax");

    char format[] = "CR0: %b \r\nCR4: %b \r\nCS: %x \r\nDS: %x\r\n";

    std_k::sprintf(string_buffer, format, cr0_info, cr4_info, cs_info, ds_info);

    io_write_s(string_buffer, IO_ports::COM_1);

    if (float_support) {
        out_byte('Y', 0x3f8);
    } else {
        out_byte('N', 0x3f8);
    }

    // Initialize Memory Managment before Constructors
    memory_init(mb_info);

    return;
}

/**
 * @brief Late Initialization function, runs before kernel_main
 *
 */
void late_init(multiboot_boot_info* mb_info) {

    threading::thread_startup_info.thread_start     = mb_info->thread_start;
    threading::thread_startup_info.thread_target    = mb_info->thread_target;
    threading::thread_startup_info.thread_stack_top = mb_info->thread_stack_top;

    // Create the kernel command line and master terminal
    kernel::cmd_init();
    log_terminal = new terminal();

    // Set up the framebuffer
    framebuffer_init(mb_info);

    // Set up boot terminals
    boot_terminal = new visual_terminal();
    set_error_terminal(new visual_terminal());
    active_terminal = boot_terminal;
    active_terminal->write_s(const_cast<char*>("PintOS Booting...\n"));
    active_terminal->update();

    // Find the ACPI RSDP
    acpi::old_rsdp* rsdp = acpi::find_rsdp(mb_info);

    // Set default interrupt handlers
    interrupts::interrupts_init(
        (acpi::madt_table*)acpi::get_table(rsdp, acpi::table_signature::MADT));

    // Detect CPU Topology
    detect_topology(
        (acpi::madt_table*)acpi::get_table(rsdp, acpi::table_signature::MADT),
        (acpi::srat_table*)acpi::get_table(rsdp, acpi::table_signature::SRAT));

    // Start system timer
    hpet* system_hpet = new hpet(
        (acpi::hpet_table*)acpi::get_table(rsdp, acpi::table_signature::HPET));
    sys_int_timer = &system_hpet->comparators[0];

    // Add cursor drawing
    std_k::function<void()>* cursor_task
        = new std_k::function<void()>(draw_active_cursor);
    sys_int_timer->push_task_rate(1, cursor_task);

    // Can now enable interrupts
    enable_interrupts();

    // Start Additional threads
    threading::start_threads();

    // Find current thread
    logical_core* thread = current_thread();

    // Setup local apic
    new (&thread->local_apic) apic<true, false>();

    // Find this core's scheduler
    threading::thread_scheduler* scheduler = thread->scheduler;

    if (scheduler == nullptr) { asm volatile("cli\n\t hlt"); }

    // Setup the scheduler
    scheduler = new (scheduler) threading::thread_scheduler(current_thread());

    // Initialization process is finished
    initialized = true;

    // Enter the boot core's scheduler
    scheduler->enter_sleep();
}
}

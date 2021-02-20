/**
 * @file kernel.c
 * @author Shane Menzies
 * @brief Basic kernel functionality
 * @date 02/07/21
 * 
 * 
 */

#include "kernel.h"

#include "display.h"
#include "multiboot.h"
#include "terminal.h"
#include "memory.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

extern uint32_t return_ebx();

/**
 * @brief If this function is called, everything stops, basically
 *        an emergency stop switch.
 * 
 */
void halt_and_catch_fire() {

    while (1) {}
}

void kernel_main() {
    struct mb_info* mb_addr = (struct mb_info*) return_ebx();

    memory_init(mb_addr);

    framebuffer_init(mb_addr);

    terminal* terminal_0 = create_terminal(0xffffff, 0x000000, 0x0f, 256000);

    char first_string[] = "Hi there, this should show up well!";

    terminal_write(terminal_0, first_string);

    char second_string[] = "Hopefully this will too though!";

    terminal_write(terminal_0, second_string);

    terminal_print(terminal_0);

    while(1) {}
}

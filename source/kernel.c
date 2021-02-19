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
//#include "memory.h"

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

terminal* terminal_0;

void kernel_main() {
    struct mb_info* mb_addr = (struct mb_info*) return_ebx();

    framebuffer_init(mb_addr);

    terminal_0->default_fg = 0xffffff;
    terminal_0->default_bg = 0x000000;
    terminal_0->default_ega = 0x0f;
    terminal_0->max_chars = 256000;
    terminal_0->index = 0;


    char first_string[] = "Hi there, this should show up well!";

    terminal_write(terminal_0, first_string);

    char second_string[] = "Hopefully this will too though!";

    terminal_write(terminal_0, second_string);

    terminal_print(terminal_0);

    while(1) {}
}

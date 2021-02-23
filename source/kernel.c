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

uint32_t* error_addr = 0;

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
    terminal_0->default_fg = 0xffffff;
    terminal_0->default_bg = 0x00UL;
    terminal_0->default_ega= 0b00001111;
    terminal_0->max_chars = 2560000;

    // Quick memory test
    for (int i = 10; i <= 2048; i += 256) {
        void* pointer = malloc(i);
        if (pointer == 0) {
            tprintf(terminal_0, "Uh Oh, something went wrong at %d", i);
        } else {
            tprintf(terminal_0, "Allocation of %d was successful", i);
            free(pointer);
        }
        tprintf(terminal_0, "After cleanup, %d marks have been used", find_null_mark(0));
        terminal_print(terminal_0);
    }

    while(1) {}
}

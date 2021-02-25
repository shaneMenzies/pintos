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

void kernel_main() {


    terminal* terminal_0 = create_terminal(0xffffff, 0x000000, 0x0f, 256000);
    terminal_0->default_fg = 0xffffff;
    terminal_0->default_bg = 0x00UL;
    terminal_0->default_ega= 0b00001111;
    terminal_0->max_chars = 2560000;


    tprintf(terminal_0, "Total Marks Used: %d\nIterations: %u\n", find_null_mark(0), 6);

    terminal_print(terminal_0);

    while(1) {}
}

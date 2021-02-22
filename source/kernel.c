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
    terminal_0->default_fg = 0xffffff;
    terminal_0->default_bg = 0x00UL;
    terminal_0->default_ega= 0b00001111;
    terminal_0->max_chars = 2560000;

    int* test_number = malloc(32);
    char test_string[] = "Test Test. This is a test";
    int decimal = -100;
    unsigned int udecimal = 100;
    unsigned int odecimal = 64;
    unsigned int xdecimal = 256;
    char test_char = '#';
    char first_string[] = "This is a test, please remain calm.\n"
                          "Decimal: %d\n"
                          "Unsigned Decimal: %u\n"
                          "Octal: %o\n"
                          "Hexadecimal: %x\n"
                          "Character: %c\n"
                          "Percent: %%\n"
                          "String: %s\n"
                          "Pointer: %p%n\n"
                          "Hurray! Printf works!";

    tprintf(terminal_0, first_string, decimal, udecimal, odecimal,
            xdecimal, test_char, test_string, test_number, test_number);

    tprintf(terminal_0, "Characters printed: %d", *test_number);

    free(test_number);

    terminal_print(terminal_0);

    while(1) {}
}

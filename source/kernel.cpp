/**
 * @file kernel.c
 * @author Shane Menzies
 * @brief Basic kernel functionality
 * @date 02/07/21
 * 
 * 
 */

#include "kernel.h"

#include "terminal.h"
#include "memory.h"
#include "init.h"
#include "error.h"

#include "display.h"
#include "multiboot.h"
#include "libk.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

struct mb_info* mb_ptr;
terminal* active_terminal;

extern "C" {
void call_kernel() {
    kernel_main();
}
}

void kernel_main() {

    //kernel_init();

    mb_ptr = (struct mb_info*) return_ebx();

    memory_init(mb_ptr);

    framebuffer_init(mb_ptr);

    terminal* terminal_0 = (new terminal());
    active_terminal = terminal_0;

    char test_string[] = "\nWell howdy there partn'r, this sure is a mighty fine day we got ourselves, now ain't it?\n";

    terminal_0->printf(const_cast<char*>("Hi there.\n%s"), test_string);

    terminal_0->show();

    delete terminal_0;

    while(1) {}
}

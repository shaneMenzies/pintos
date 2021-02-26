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

    mb_ptr = (struct mb_info*) 0x10000;

    memory_init(mb_ptr);

    framebuffer_init((struct mb_info*) 0x10000);

    terminal* terminal_0 = (new terminal(0xffffff, 0x000000, 0x0f));
    terminal_0->default_fg = 0xffffff;
    terminal_0->default_bg = 0;
    terminal_0->default_ega = 0x0f;
    active_terminal = terminal_0;

    terminal_0->printf(const_cast<char*>("Hi there, Agent %d"), 47);

    terminal_0->show();

    delete terminal_0;

    while(1) {}
}

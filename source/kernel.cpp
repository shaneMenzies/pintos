/**
 * @file kernel.cpp
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

extern "C" {
void call_kernel() {
    kernel_main();

}
    extern void test_soft_int();
}

void kernel_main() {

    active_terminal->clear();

    char test_string[] = "\nWell howdy there partn'r, this sure is a mighty fine day we got ourselves, now ain't it?\n\0";

    active_terminal->tprintf(const_cast<char*>("Hi there.\n%s"), test_string);

    active_terminal->show();

    v_fb test_fb;
    test_fb.draw_rect_fill(100, 100, 200, 200, 0xffffffff);
    uint32_t x = 4;
    uint32_t y = 4;
    test_fb.draw_s(x, y, "Test test. This is a test. Everything is okay.", 0xffffff, 0, 0x0f);
    test_fb.show();

    while(1) {}
}

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
#include "p_memory.h"
#include "init.h"
#include "error.h"
#include "timer.h"

#include "display.h"
#include "libk.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

terminal* log_terminal;

void kernel_main() {

    active_terminal->clear();

    char test_string[] = "\nWell howdy there partn'r, this sure is a mighty fine day we got ourselves, now ain't it?\n\0";

    active_terminal->tprintf(const_cast<char*>("Hi there.\n%s"), test_string);

    if (float_support) {
        active_terminal->write_s("Floating Point Instruction Support Enabled\n");
    } else {
        active_terminal->write_s("Floating Point Instructions not supported\n");
    }

    active_terminal->update();

    while(1) {}
}

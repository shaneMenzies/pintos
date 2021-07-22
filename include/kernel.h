#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include "terminal.h"

extern "C" {
void call_kernel();
}

extern terminal* log_terminal;

void kernel_main();

#endif
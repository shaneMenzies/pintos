#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

enum {
    POINTER_MASK = ~(1ULL << (sizeof(void*) * 8))
};

void kernel_main();

#endif
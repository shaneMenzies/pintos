/**
 * @file libk.c
 * @author Shane Menzies
 * @brief Some freestanding C standard library functions
 * @date 02/14/21
 * 
 * 
 */

#include "libk.h"

#include <stddef.h>

void* memcpy(void* __restrict__ dest_ptr, const void* __restrict__ src_ptr, 
             size_t size) {

    unsigned char* dest = (unsigned char*) dest_ptr;
    const unsigned char* source = (unsigned char*) src_ptr;

    for (size_t i = 0; i < size; i++) {
        dest[i] = source[i];
    }

    return dest_ptr;
}

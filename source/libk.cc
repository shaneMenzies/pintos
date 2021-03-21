/**
 * @file libk.cc
 * @author Shane Menzies
 * @brief Some freestanding C standard library functions
 * @date 02/14/21
 * 
 * 
 */

#include "libk.h"

#include <stddef.h>

/**
 * @brief Copies the contents of one area of memory to another
 * 
 * @param dest_ptr  Pointer to the start of the destination to be copied to
 * @param src_ptr   Pointer to the start of the source to be copied from
 * @param size      Size (in bytes) of area to be copied
 * @return void*    dest_ptr
 */
void* memcpy(void* __restrict__ dest_ptr, const void* __restrict__ src_ptr, 
             size_t size) {

    unsigned char* dest = (unsigned char*) dest_ptr;
    const unsigned char* source = (unsigned char*) src_ptr;

    for (size_t i = 0; i < size; i++) {
        dest[i] = source[i];
    }

    return dest_ptr;
}

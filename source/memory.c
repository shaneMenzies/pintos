/**
 * @file memory.c
 * @author Shane Menzies
 * @brief Functions for memory management
 * @date 02/14/21
 * 
 * 
 */

#include "memory.h"

#include "multiboot.h"

#include <stddef.h>
#include <stdint.h>

// STACK-BASED MEMORY

void* memory_sp = 0;
void* memory_sp_min = 0;
void* memory_sp_max = 0;

void memory_init(struct mb_info* mb_addr) {

    memory_sp = memory_sp_min = (void*)mb_addr->mem_lower;
    memory_sp_max = (void*)mb_addr->mem_upper;

}

void* malloc(size_t size) {

    void* return_pointer = memory_sp;
    memory_sp += size;

    if (memory_sp >= memory_sp_max) {
        memory_sp = memory_sp_min;
        return malloc(size);
    } else {
        return return_pointer;
    }
}

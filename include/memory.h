#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

#include "bookmark.h"
#include "trees.h"
#include "multiboot.h"
#include "error.h"

extern void* max_phys_address;

trees::bookmark* mkalloc();

trees::bookmark mmap_to_mark(uint32_t* mmap_addr);

void memory_init(struct mb_info* mb_addr);

void* malloc(size_t size);

void* talloc(void* target_address, size_t size);

void free(void* target_address);

#endif
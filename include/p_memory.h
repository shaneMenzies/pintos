#ifndef P_MEMORY_H
#define P_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#include "multiboot.h"
#include "bookmark.h"
#include "trees.h"
#include "error.h"
#include "paging.h"

extern void* max_phys_address;

trees::bookmark* mkalloc();

trees::bookmark mmap_to_mark(multiboot_mmap_entry* mmap);

void memory_init(multiboot_boot_info* mb_info);

void* malloc(size_t size);

void* balloc(size_t size, size_t alignment);

void* talloc(void* target_address, size_t size);

void free(void* target_address);

#endif
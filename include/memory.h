#ifndef MEMORY_H
#define MEMORY_H

#include "multiboot.h"

#include <stddef.h>
#include <stdint.h>

typedef struct bookmark {

    void* start;
    void* end;
    bool available_flag;

} bookmark;

bookmark mmap_to_mark(uint32_t* mmap_addr);

int find_null_mark(int start);

void cycle_marks_up(int start, int count);

void cycle_marks_down(int start, int count);

void memory_init(struct mb_info* mb_addr);

void* malloc(size_t size);

void* talloc(void* target_address, size_t size);

void free(void* target_address);

#endif
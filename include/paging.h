#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include "p_memory.h"

namespace paging {

const uint64_t page_size = 0x1000;
const uint64_t page_table_size = (page_size * 512);
const uint64_t page_directory_size = (page_table_size * 512);
const uint64_t page_directory_pointer_size = (page_directory_size * 512);
const uint64_t page_level_4_table_size = (page_directory_pointer_size * 512);
const uintptr_t real_address_bitmask = ~(0xfff);

typedef uint64_t page_entry;
typedef struct { page_entry data[512]; } page_table;

typedef uint64_t page_table_entry;
typedef struct { page_table_entry data[512]; } page_directory_table;

typedef uint64_t page_directory_entry;
typedef struct { page_directory_entry data[512]; } page_directory_pointer_table;

typedef uint64_t page_directory_pointer_table_entry;
typedef struct { page_directory_pointer_table_entry data[512]; } page_level_4_table;

enum page_flags {
    p_present = (1 << 0),
    p_write_enabled = (1 << 1),
    p_supervisor_restricted = (1 << 2),
    p_write_through = (1 << 3),
    p_cache_disabled = (1 << 4),
    p_accessed = (1 << 5),

    p_dirty = (1 << 6),
    p_global = (1 << 8)
};

enum page_table_flags {
    pt_present = (1 << 0),
    pt_write_enabled = (1 << 1),
    pt_supervisor_restricted = (1 << 2),
    pt_write_through = (1 << 3),
    pt_cache_disabled = (1 << 4),
    pt_accessed = (1 << 5),

    pt_large_pages = (1 << 7),
};

enum page_directory_flags {
    pd_present = (1 << 0),
    pd_write_enabled = (1 << 1)
};

enum page_directory_pointer_flags {
    pdp_present = (1 << 0),
    pdp_write_enabled = (1 << 1)
};

inline page_level_4_table* get_current_table() {

    uint32_t cr3_value;

    asm volatile (
        "movq %%cr3, %%rax\n\t\
         movl %%eax, %[value]"
        : [value] "=r" (cr3_value)
        :
        : "rax"
    );

    return (page_level_4_table*) (uintptr_t)(cr3_value | 0UL);
};

inline void refresh_all_pages() {
    asm volatile (
        "movq %%cr3, %%rax\n\t\
         movq %%rax, %%cr3"
        :
        :
        : "rax"
    );
};

inline void refresh_page(void* target) {
    asm volatile (
        "invlpg %[target]"
        : 
        : [target] "m" (target)
    );
};

void identity_map_page(uintptr_t physical_address);
void identity_map_pt(uintptr_t physical_address);
void identity_map_pd(uintptr_t physical_address);

void identity_map_region(uintptr_t target_address, size_t size);

}

#endif
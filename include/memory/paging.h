#ifndef PAGING_H
#define PAGING_H

#include "memory/p_memory.h"

#include <stddef.h>
#include <stdint.h>

namespace paging {

constexpr uint64_t page_table_size             = (PAGE_SIZE * 512);
constexpr uint64_t page_directory_size         = (page_table_size * 512);
constexpr uint64_t page_directory_pointer_size = (page_directory_size * 512);
constexpr uint64_t page_level_4_table_size
    = (page_directory_pointer_size * 512);
constexpr uintptr_t real_address_bitmask = ~(0xfff);

typedef uint64_t page_entry;
typedef struct {
    page_entry data[512];
} page_table;

typedef uint64_t page_table_entry;
typedef struct {
    page_table_entry data[512];
} page_directory_table;

typedef uint64_t page_directory_entry;
typedef struct {
    page_directory_entry data[512];
} page_directory_pointer_table;

typedef uint64_t page_directory_pointer_table_entry;
struct page_level_4_table {
    page_directory_pointer_table_entry data[512];
};

constexpr uintptr_t       table_start_address = 0xffffff7fbfdff000;
page_level_4_table* const pml4_table
    = (page_level_4_table*)table_start_address; // 1 pml4 table
page_directory_pointer_table* const pdp_tables
    = (page_directory_pointer_table*)(table_start_address
                                      + PAGE_SIZE); // 512 pdp tables
page_directory_table* const pd_tables
    = (page_directory_table*)(table_start_address
                              + (PAGE_SIZE * 513)); // 262144 pd tables
page_table* const pt_tables
    = (page_table*)(table_start_address + (PAGE_SIZE * ((512 * 512) + 513)));

enum page_flags {
    p_present               = (1 << 0),
    p_write_enabled         = (1 << 1),
    p_supervisor_restricted = (1 << 2),
    p_write_through         = (1 << 3),
    p_cache_disabled        = (1 << 4),
    p_accessed              = (1 << 5),

    p_dirty  = (1 << 6),
    p_global = (1 << 8)
};

enum page_table_flags {
    pt_present               = (1 << 0),
    pt_write_enabled         = (1 << 1),
    pt_supervisor_restricted = (1 << 2),
    pt_write_through         = (1 << 3),
    pt_cache_disabled        = (1 << 4),
    pt_accessed              = (1 << 5),

    pt_large_pages = (1 << 7),
};

enum page_directory_flags {
    pd_present       = (1 << 0),
    pd_write_enabled = (1 << 1)
};

enum page_directory_pointer_flags {
    pdp_present       = (1 << 0),
    pdp_write_enabled = (1 << 1)
};

inline void* round_up_page_aligned(void* target) {
    return (void*)(((uintptr_t)target + PAGE_SIZE - 1) & (-PAGE_SIZE));
}
inline void* round_down_page_aligned(void* target) {
    return (void*)((uintptr_t)target - ((uintptr_t)target % PAGE_SIZE));
}

inline size_t round_up_page_aligned(size_t target) {
    return ((target + PAGE_SIZE - 1) & (-PAGE_SIZE));
}
inline size_t round_down_page_aligned(size_t target) {
    return (target - (target % PAGE_SIZE));
}

inline uintptr_t get_current_table() {

    uint64_t cr3_value;

    asm volatile("movq %%cr3, %%rax\n\t\
         movq %%rax, %[value]"
                 : [value] "=r"(cr3_value)
                 :
                 : "rax");

    return (uintptr_t)(cr3_value | 0UL);
};

inline void refresh_all_pages() {
    asm volatile("movq %%cr3, %%rax\n\t\
         movq %%rax, %%cr3"
                 :
                 :
                 : "rax");
};

inline void refresh_page(void* target) {
    asm volatile("invlpg %[target]" : : [target] "m"(target));
};
} // namespace paging

#endif
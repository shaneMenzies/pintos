#ifndef P_MEMORY_H
#define P_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE (size_t)0x1000

struct multiboot_boot_info;
struct multiboot_mmap_entry;

#define NUM_PROTECTED_REGIONS 7
extern void* protected_regions[NUM_PROTECTED_REGIONS][2];

namespace std {
enum class align_val_t : size_t {};
}
using std::align_val_t;

void* operator new(size_t size);
void* operator new(size_t size, align_val_t alignment);
void* operator new(size_t size, void* placement);
void* operator new[](size_t size);
void* operator new[](size_t size, align_val_t alignment);
void* operator new[](size_t size, void* placement);

void operator delete(void* p);
void operator delete(void* p, align_val_t alignment);
void operator delete(void* p, size_t size);
void operator delete(void* p, size_t size, align_val_t alignment);
void operator delete[](void* p);
void operator delete[](void* p, align_val_t alignment);
void operator delete[](void* p, size_t size);
void operator delete[](void* p, size_t size, align_val_t alignment);

void fill_protected_regions(multiboot_boot_info* mb_info);
int  check_protected_regions(void* added_start, size_t added_size,
                             int start_check_at);

void memory_init(multiboot_boot_info* mb_info);

void* bootstrap_malloc(size_t size);
void* malloc(size_t size);
void* lock_override_alloc(size_t size, int override_lock_index = -1);
void* aligned_alloc(size_t size, size_t alignment);

uintptr_t bootstrap_palloc(bool lock_override = false);
uintptr_t palloc(bool lock_override = false);

void free(void* target_address);
void pfree(uintptr_t physical_address);
void lock_override_free(void* target_address, int override_lock_index = -1);

inline bool cmp_swap(uint64_t* target, uint64_t cmp_value, uint64_t new_value) {
    uint64_t result;
    asm volatile("lock cmpxchgq %[value], (%%rcx) \n\t\
                   movq %%rax, %[result]"
                 : [result] "=m"(result)
                 : [target] "c"(target),
                   "a"(cmp_value), [value] "d"(new_value));

    return (result == cmp_value);
}

#endif

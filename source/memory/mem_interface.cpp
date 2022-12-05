/**
 * @file memory.cpp
 * @author Shane Menzies
 * @brief Functions for memory management
 * @date 02/14/21
 *
 *
 */

#include "addressing.h"
#include "allocation_manager.h"
#include "chunking.h"
#include "p_memory.h"
#include "sub_memory.h"
#include "super_memory.h"
#include "system/init.h"
#include "system/pintos_std.h"
#include "threading/threading.h"

extern "C" {
extern void* kernel_start;
extern void* kernel_end;
}

allocation_manager kernel_allocation_manager;

void* operator new(size_t size) { return malloc(size); }
void* operator new[](size_t size) { return malloc(size); }

void* operator new(size_t size, void* placement) {
    (void)size;
    return placement;
}
void* operator new[](size_t size, void* placement) {
    (void)size;
    return placement;
}

void operator delete(void* p) { free(p); }
void operator delete(void* p, long unsigned int size) {
    (void)size;
    free(p);
}

void operator delete[](void* p) { free(p); }
void operator delete[](void* p, long unsigned int size) {
    (void)size;
    free(p);
}

void* protected_regions[NUM_PROTECTED_REGIONS][2];

void fill_protected_regions(multiboot_boot_info* mb_info) {

    // Kernel Space
    protected_regions[0][0] = &kernel_start;
    protected_regions[0][1]
        = (void*)((uintptr_t)&kernel_end - (uintptr_t)&kernel_start);

    // Stack
    protected_regions[1][0] = mb_info->stack_bottom;
    protected_regions[1][1] = (void*)((uintptr_t)mb_info->stack_top
                                      - (uintptr_t)mb_info->stack_bottom);

    // Boot data
    protected_regions[2][0] = mb_info->boot_start;
    protected_regions[2][1] = (void*)mb_info->boot_size;

    // Multiboot info
    protected_regions[3][0] = mb_info->mb_start;
    protected_regions[3][1] = (void*)mb_info->mb_size;

    // Additional thread startup code
    protected_regions[4][0] = mb_info->thread_start;
    protected_regions[4][1] = (void*)mb_info->thread_size;

    // 0-32 for Error handling
    protected_regions[5][0] = 0;
    protected_regions[5][1] = (void*)32;

    // Framebuffer
    protected_regions[6][0]
        = (void*)mb_info->framebuffer_tag->common.framebuffer_addr;
    protected_regions[6][1]
        = (void*)((uintptr_t)mb_info->framebuffer_tag->common.framebuffer_pitch
                  * mb_info->framebuffer_tag->common.framebuffer_height);
}

int check_protected_regions(void* added_start, size_t added_size,
                            int start_check_at) {
    if (start_check_at >= NUM_PROTECTED_REGIONS) { return -1; }

    void* added_end = (void*)((uintptr_t)added_start + added_size - 1);

    // Compare what's been added to each protected region
    for (unsigned int i = start_check_at; i < NUM_PROTECTED_REGIONS; i++) {
        void* protected_start = protected_regions[i][0];
        void* protected_end   = (void*)((uintptr_t)protected_start
                                      + (uintptr_t)protected_regions[i][1] - 1);

        if ((added_start <= protected_end) && (added_end >= protected_start)) {
            return i;
        }
    }

    return -1;
}

void memory_init(multiboot_boot_info* mb_info) {

    fill_protected_regions(mb_info);

    chunking::construct_chunks(mb_info);

    paging::kernel_address_space.initialize();
}

/**
 * @brief Allocates an area of memory the size of size in bytes
 *
 * @param size      Bytes to allocate
 * @return void*    Pointer to start of newly allocated memory
 */
void* malloc(size_t size) {

    if (size == 0) { return 0; }

    // Hand real allocation to appropriate handler
    if (size < SUB_PAGE_DATA_PER_PAGE) {
        return paging::kernel_address_space.sub_page_memory.sub_alloc(size);
    } else {
        allocation_entry* return_entry;
        if (initialized) {
            return_entry = chunk_alloc(size);
        } else {
            return_entry = bootstrap_chunk_alloc(size);
        }

        kernel_allocation_manager.add_entry(return_entry);

        return (void*)return_entry->value.address;
    }
}

/**
 * @brief Allocates an area of memory the size of size in bytes
 *
 * @param size      Bytes to allocate
 * @return void*    Pointer to start of newly allocated memory
 */
void* lock_override_alloc(size_t size, int override_lock_index) {

    if (size == 0) { return 0; }

    // Hand real allocation to appropriate handler
    allocation_entry* return_entry
        = override_chunk_alloc(size, override_lock_index);

    kernel_allocation_manager.add_entry(return_entry);

    return (void*)return_entry->value.address;
}

/**
 * @brief Allocates an area of memory the size of size in bytes, aligned
 *          on an alignment-byte boundary
 *
 * @param size      Bytes to allocate
 * @param alignment Needed alignment in bytes
 * @return void*    Pointer to start of newly allocated memory
 */
void* aligned_alloc(size_t size, size_t alignment) {

    if (size == 0) { return 0; }

    // Hand real allocation to appropriate handler
    if (size < SUB_PAGE_DATA_PER_PAGE && alignment < (PAGE_SIZE / 2)) {
        return paging::kernel_address_space.sub_page_memory.sub_aligned_alloc(
            size, alignment);
    } else {
        allocation_entry* return_entry;
        if (initialized) {
            return_entry = aligned_chunk_alloc(size, alignment);
        } else {
            return_entry = bootstrap_aligned_chunk_alloc(size, alignment);
        }

        kernel_allocation_manager.add_entry(return_entry);

        return (void*)return_entry->value.address;
    }
}

uintptr_t bootstrap_palloc(bool lock_override) {

    // Get the required chunk
    chunking::chunk required_chunk
        = chunking::memory_reservoirs[0].get_chunk(lock_override);
    return required_chunk.p_start;
}

uintptr_t palloc(bool lock_override) {

    if (!initialized) { return (uintptr_t)bootstrap_palloc(lock_override); }

    // Start at piles
    chunking::chunk_pile* current_piles = current_thread()->memory_piles;

    // Get the required chunk
    chunking::chunk required_chunk
        = current_piles[0].get_chunk(false, lock_override);
    return required_chunk.p_start;
}

/**
 * @brief Frees the area of memory starting at the provided address
 *
 * @param target_address    Address of the start of the area to be freed
 */
void free(void* target_address) {

    // Check with sub-page bounds first
    if (!paging::kernel_address_space.sub_page_memory.try_sub_free(
            (uintptr_t)target_address)) {
        // Free returned chunks (super-page allocation)
        allocation_info returned_entry
            = kernel_allocation_manager.take_entry((uintptr_t)target_address);
        chunk_free(returned_entry);
    }
}

void pfree(uintptr_t physical_address) {
    chunking::chunk return_chunk;
    return_chunk.size    = 0x1000;
    return_chunk.p_start = physical_address;
    chunking::memory_reservoirs[0].add_chunk(return_chunk);
}

void lock_override_free(void* target_address, int override_lock_index) {
    allocation_info returned_entry
        = kernel_allocation_manager.take_entry((uintptr_t)target_address);
    override_chunk_free(returned_entry, override_lock_index);
}

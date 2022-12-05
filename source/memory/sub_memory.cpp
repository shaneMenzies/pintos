/**
 * @file sub_memory.cpp
 * @author Shane Menzies
 * @brief Sub-page memory management
 * @date 01/03/22
 *
 *
 */

#include "sub_memory.h"

#include "system/pintos_std.h"

sub_mem_manager kernel_sub_mem_manager;

void* sub_mem_manager::sub_alloc(size_t size) {
    sub_mem_region* current_region = &first_region;

    // Check existing regions
    while (1) {
        if (current_region->total_space_free > size) {
            void* returned_address = current_region->allocate_space(size);
            if ((uintptr_t)returned_address != 0) { return returned_address; }
        }

        if ((uintptr_t)current_region->next_region) {
            current_region = current_region->next_region;
        } else {
            break;
        }
    }

    // Need to add another region to allocate on
    current_region->next_region = new sub_mem_region();
    if ((uintptr_t)current_region->next_region < earliest_address)
        earliest_address = (uintptr_t)current_region->next_region;
    if ((uintptr_t)current_region->next_region + PAGE_SIZE > latest_address)
        latest_address = (uintptr_t)current_region->next_region + PAGE_SIZE;
    return current_region->next_region->allocate_space(size);
}

void* sub_mem_manager::sub_aligned_alloc(size_t size, size_t alignment) {
    sub_mem_region* current_region = &first_region;

    // Check existing regions
    while (1) {
        if (current_region->total_space_free > size) {
            void* returned_address
                = current_region->allocate_aligned_space(size, alignment);
            if ((uintptr_t)returned_address != 0) { return returned_address; }
        }

        if ((uintptr_t)current_region->next_region) {
            current_region = current_region->next_region;
        } else {
            break;
        }
    }

    // Need to add another region to allocate on
    current_region->next_region = new sub_mem_region();
    if ((uintptr_t)current_region->next_region < earliest_address)
        earliest_address = (uintptr_t)current_region->next_region;
    if ((uintptr_t)current_region->next_region + PAGE_SIZE > latest_address)
        latest_address = (uintptr_t)current_region->next_region + PAGE_SIZE;
    return current_region->next_region->allocate_aligned_space(size, alignment);
}

bool sub_mem_manager::try_sub_free(uintptr_t address) {
    // Check rough bounds
    if (address >= earliest_address && address <= latest_address) {
        // Check with each region
        sub_mem_region* current_region = &first_region;
        while (1) {
            if (address >= (uintptr_t)current_region
                && address <= ((uintptr_t)current_region + PAGE_SIZE)) {
                current_region->free_space((void*)address);
                return true;
            }

            if ((uintptr_t)current_region->next_region)
                current_region = current_region->next_region;
            else
                return false;
        }
    } else {
        return false;
    }
}
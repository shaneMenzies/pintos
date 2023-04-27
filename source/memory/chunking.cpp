/**
 * @file chunking.cpp
 * @author Shane Menzies
 * @brief PintOS Memory Management using chunks
 * @date 08/25/21
 *
 *
 */

#include "chunking.h"

#include "paging.h"
#include "system/init.h"
#include "system/multiboot.h"
#include "system/pintos_std.h"

namespace chunking {

chunk reservoir_chunks[NUM_MEMORY_PILES][RESERVOIR_DEFAULT_SIZE];

chunk_reservoir memory_reservoirs[NUM_MEMORY_PILES];

chunk chunk_reservoir::get_chunk(bool lock_override) {

    if (!lock_override) access_lock.lock();

    chunk return_value;
    if ((uintptr_t)last >= (uintptr_t)start) {
        return_value = *last;
        last         = (chunk*)((uintptr_t)last - sizeof(chunk));
        if (!lock_override) access_lock.unlock();

    } else if (pile_index < (NUM_MEMORY_PILES - 1)) {
        if (!lock_override) access_lock.unlock();

        // Get chunk from next largest reservoir to fill this one
        chunk break_down_chunk = memory_reservoirs[pile_index + 1].get_chunk();

        if (break_down_chunk.size == 0) {
            if (!initialized) { return chunk(0, 0); }

            // None available in reservoir, search other threads' piles
            unsigned int thread = 0;
            while (1) {
                chunk_pile* target_pile
                    = &topology.threads[thread].memory_piles[pile_index + 1];
                break_down_chunk = target_pile->get_chunk();
                if (break_down_chunk.size != 0) { break; }

                thread++;
                if (thread == topology.num_logical) { return chunk(0, 0); }
            }
        }

        // Break it down into smaller chunks
        chunk new_chunks[0x10];
        for (int i = 0; i < 0x10; i++) {
            new_chunks[i].p_start = break_down_chunk.p_start;
            new_chunks[i].size    = chunk_size;

            break_down_chunk.p_start += chunk_size;
        }

        add_chunks(new_chunks, 0xf, lock_override);
        return_value = new_chunks[0xf];
    } else {
        if (!lock_override) access_lock.unlock();

        return chunk(0, 0);
    }

    return return_value;
}

void turn_to_chunks(uintptr_t start_address, size_t size) {

    // Start at page-aligned address
    uintptr_t aligned_address
        = (uintptr_t)paging::round_up_page_aligned((void*)start_address);
    size -= (aligned_address - start_address);
    start_address = aligned_address;

    if (size < PAGE_SIZE) { return; }

    // Can turn this area of memory into chunks
    uint64_t     current_size  = PAGE_SIZE << ((NUM_MEMORY_PILES - 1) * 4);
    unsigned int current_index = NUM_MEMORY_PILES - 1;
    while (size >= PAGE_SIZE && current_size >= PAGE_SIZE) {

        unsigned int num_chunks = size / current_size;
        size %= current_size;
        while (num_chunks) {
            // Create next chunk
            chunk new_chunk(current_size, start_address);

            // Add it to the appropriate reservoir
            memory_reservoirs[current_index].add_chunk(new_chunk);

            // Increase the target's address
            start_address += current_size;
            num_chunks--;
        }

        current_size >>= 4;
        current_index--;
    }
}

void mmap_to_chunks(multiboot_mmap_entry target) {

    // Only break down into chunks if its available memory
    if (target.type != MULTIBOOT_MEMORY_AVAILABLE || target.len == 0) {
        return;
    }

    // Check for any conflict with protected regions
    int conflicting_region
        = check_protected_regions((void*)target.addr, target.len, 0);
    while (conflicting_region >= 0) {

        // Create end slice for area of target after the protected region
        uint64_t             target_end = (uint64_t)target.addr + target.len;
        multiboot_mmap_entry end_slice;
        end_slice.addr = ((uint64_t)protected_regions[conflicting_region][0]
                          + (uint64_t)protected_regions[conflicting_region][1]);
        end_slice.len
            = (target_end > end_slice.addr) ? (target_end - end_slice.addr) : 0;
        end_slice.type = target.type;
        mmap_to_chunks(end_slice);

        // Adjust target to be first slice before continuing
        target.len
            = (size_t)(((target.addr
                         < (uint64_t)protected_regions[conflicting_region][0]))
                           ? ((uint64_t)protected_regions[conflicting_region][0]
                              - target.addr)
                           : 0);

        if (target.len == 0) { return; }

        // Check adjusted slice, comparing with rest of protected_regions
        conflicting_region
            = check_protected_regions((void*)target.addr, target.len,
                                      (unsigned int)(conflicting_region + 1));
    }

    // Can turn this area of memory into chunks
    turn_to_chunks(target.addr, target.len);
}

void construct_chunks(multiboot_boot_info* mb_info) {

    // Create the reservoirs
    for (unsigned int i = 0; i < NUM_MEMORY_PILES; i++) {
        new (&memory_reservoirs[i]) chunk_reservoir(i);
    }

    // Break the multiboot entries down into the chunk reservoirs
    multiboot_mmap_entry* next_mmap = (&mb_info->mmap_tag->entries[0]);
    uintptr_t mmap_end = (uintptr_t)mb_info->mmap_tag + mb_info->mmap_tag->size;
    size_t    entry_size = mb_info->mmap_tag->entry_size;
    while (1) {

        // Process entry
        mmap_to_chunks(*next_mmap);

        // Move to the next entry, if it exists
        next_mmap = (multiboot_mmap_entry*)((uintptr_t)next_mmap + entry_size);
        if ((uintptr_t)next_mmap >= mmap_end) { break; }
    }

    // TODO: Identity map the protected regions
}
} // namespace chunking

#ifndef CHUNKING_H
#define CHUNKING_H

#include "addressing.h"
#include "libk/callable.h"
#include "libk/functional.h"
#include "memory/chunking_predef.h"
#include "memory/p_memory.h"
#include "memory/paging.h"
#include "system/error.h"
#include "system/pintos_std.h"
#include "threading/threading.h"

#include <stddef.h>
#include <stdint.h>

struct multiboot_boot_info;
struct multiboot_mmap_entry;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winit-self"
#pragma GCC diagnostic ignored "-Wuninitialized"

namespace chunking {

struct chunk {
    size_t    size;
    uintptr_t p_start;

    chunk()
        : size(size)
        , p_start(p_start) {}
    chunk(size_t size, uintptr_t p_start)
        : size(size)
        , p_start(p_start) {}

    void* map_to(void* target) {
        paging::kernel_address_space.map_region_to(p_start, (uintptr_t)target,
                                                   size);
        return (void*)((uintptr_t)target + size);
    }

    void* lock_override_map_to(void* target, int lock_override_index) {
        paging::kernel_address_space.map_region_to(
            p_start, (uintptr_t)target, size, (lock_override_index == 0));
        return (void*)((uintptr_t)target + size);
    }
};

struct chunk_reservoir {
    unsigned int pile_index;
    size_t       chunk_size;

    chunk* start;
    chunk* last;
    chunk* end;

    bool access_lock;
    bool can_free_reservoir;

    chunk_reservoir()
        : pile_index(pile_index)
        , chunk_size(chunk_size)
        , start(start)
        , last(last)
        , end(end)
        , access_lock(access_lock)
        , can_free_reservoir(can_free_reservoir) {}
    chunk_reservoir(unsigned int pile_index)
        : pile_index(pile_index)
        , access_lock(false)
        , can_free_reservoir(false) {
        chunk_size = (PAGE_SIZE << (pile_index * 4));

        start = &reservoir_chunks[pile_index][1];
        last  = &reservoir_chunks[pile_index][0];
        end   = &reservoir_chunks[pile_index + 1][0];
    }

    chunk get_chunk(bool lock_override = false);

    void get_chunks(int num_chunks, chunk* target_buffer,
                    bool lock_override = false) {

        if (!lock_override) get_lock(&access_lock);

        for (int i = 0; i < num_chunks; i++) {
            target_buffer[i] = get_chunk(true);
        }

        if (!lock_override) access_lock = false;

        return;
    }

    void add_chunk(chunk new_chunk, bool lock_override = false) {

        if (new_chunk.size == 0) { return; }

        if (!lock_override) { get_lock(&access_lock); }

        // Calculate new position
        chunk* new_position = (chunk*)((uintptr_t)last + sizeof(chunk));

        // Check if this would be a valid location
        if ((uintptr_t)new_position > (uintptr_t)end) {
            // Get new, larger array
            unsigned int current_count
                = (((uintptr_t)end - (uintptr_t)start) / sizeof(chunk));

            chunk* new_start
                = (chunk*)lock_override_alloc((sizeof(chunk) * 2), pile_index);
            chunk* new_end  = &new_start[current_count * 2];
            chunk* new_last = (chunk*)((uintptr_t)new_start
                                       + ((uintptr_t)last - (uintptr_t)start));

            chunk* source = start;
            chunk* dest   = new_start;
            while ((uintptr_t)dest < (uintptr_t)new_last) {
                *dest  = *source;
                source = (chunk*)((uintptr_t)source + sizeof(chunk));
                dest   = (chunk*)((uintptr_t)dest + sizeof(chunk));
            }

            start = new_start;
            end   = new_end;
            last  = new_last;

            if (can_free_reservoir) {
                lock_override_free(start, pile_index);
            } else {
                can_free_reservoir = true;
            }
        }

        // Put this chunk in the new position
        last  = (chunk*)((uintptr_t)last + sizeof(chunk));
        *last = new_chunk;

        if (!lock_override) { access_lock = false; }
    }

    void add_chunks(chunk* new_chunks, int num_chunks,
                    bool lock_override = false) {

        if (!lock_override) get_lock(&access_lock);

        for (int i = 0; i < num_chunks; i++) { add_chunk(new_chunks[i], true); }

        if (!lock_override) access_lock = false;
    }
};

inline unsigned int get_chunk_index(size_t size) {
    for (unsigned int i = 0; i < NUM_MEMORY_PILES; i++) {
        if (size == get_chunk_size(i)) { return i; }
    }

    return (NUM_MEMORY_PILES - 1);
};

inline unsigned int get_num_chunks(size_t size,
                                   bool   include_chunk_sizes = false) {
    unsigned int sum = 0;

    if (size % PAGE_SIZE) { size += PAGE_SIZE; }
    size /= PAGE_SIZE;

    while (size) {
        sum += (size & 0b1111);
        size >>= 4;
    }

    if (include_chunk_sizes) {
        sum += get_num_chunks((sum * sizeof(chunk)), false);
    }

    return sum;
};

struct chunk_pile {

    unsigned int pile_index = 0;
    size_t       chunk_size = 0;

    chunk               chunks[CHUNKS_PER_PILE];
    int                 next_chunk          = -1;
    threading::process* refresh_task        = 0;
    bool                refresh_task_active = false;
    bool                access_lock         = false;

    chunk_pile() {};
    chunk_pile(unsigned int pile_index)
        : pile_index(pile_index) {

        // Set chunk size
        chunk_size = (PAGE_SIZE << (pile_index * 4));

        // Fill chunks from reservoir
        for (unsigned int i = 0; i < CHUNKS_PER_PILE; i++) {
            chunks[i] = memory_reservoirs[pile_index].get_chunk();
            if (chunks[i].size) { next_chunk = i; }
        }
    }

    static void refill_chunks(chunk_pile& target) {

        get_lock(&target.access_lock);

        while (target.next_chunk < (int)(CHUNKS_PER_PILE - 1)) {
            target.next_chunk++;
            target.chunks[target.next_chunk]
                = memory_reservoirs[target.pile_index].get_chunk();
            if (target.chunks[target.next_chunk].size == 0) {
                target.next_chunk--;
                break;
            }
        }

        // delete refresh_task
        target.refresh_task_active = false;
        target.access_lock         = false;
    }

    chunk get_chunk(bool pile_lock_override      = false,
                    bool reservoir_lock_override = false) {

        if (!pile_lock_override) { get_lock(&access_lock); }

        // Get the next chunk
        chunk value;
        if (next_chunk > -1) {
            // Try to grab from this pile
            value = chunks[next_chunk];
            next_chunk--;

            if (!pile_lock_override) { access_lock = false; }
        } else {
            if (!pile_lock_override) { access_lock = false; }

            // Try to grab from reservoir
            value = memory_reservoirs[pile_index].get_chunk(
                reservoir_lock_override);

            if (value.size == 0) {
                // None available in reservoir, search other threads' piles
                for (unsigned int thread = 0; thread < topology.num_logical;
                     thread++) {
                    chunk_pile* target_pile
                        = &topology.threads[0].memory_piles[pile_index];
                    if (target_pile != this) {
                        value = target_pile->get_chunk();
                        if (value.size != 0) { break; }
                    }
                }

                if (value.size == 0) {
                    // None found
                    return chunk(0, 0);
                }
            }
        }

        // Send a thread to refill chunks
        if (!pile_lock_override && next_chunk < (int)(CHUNKS_PER_PILE / 4)
            && !refresh_task_active) {
            std_k::preset_function<typeof(refill_chunks)> task(refill_chunks,
                                                               *this);
            if (try_lock(&refresh_task_active)) {
                refresh_task = new threading::process(2, 4, 1, 0x100, &task);
                threading::main_scheduler.send_task(refresh_task);
            }
        }

        return value;
    }

    void get_chunks(int num_chunks, chunk* target_buffer,
                    bool pile_lock_override      = false,
                    bool reservoir_lock_override = false) {

        if (!pile_lock_override) get_lock(&access_lock);

        for (int i = 0; i < num_chunks; i++) {
            target_buffer[i] = get_chunk(true, reservoir_lock_override);
        }

        // Send a thread to refill chunks
        if (!pile_lock_override && next_chunk < (int)(CHUNKS_PER_PILE / 4)
            && !refresh_task_active) {
            std_k::preset_function<typeof(refill_chunks)> task(refill_chunks,
                                                               *this);
            if (try_lock(&refresh_task_active)) {
                refresh_task = new threading::process(2, 4, 1, 0x100, &task);
                threading::main_scheduler.send_task(refresh_task);
            }
        }

        return;
    }
};

void turn_to_chunks(uintptr_t start_address, size_t size);
void construct_chunks(multiboot_boot_info* mb_info);
void mmap_to_chunks(multiboot_mmap_entry target);
} // namespace chunking

#pragma GCC diagnostic pop

#endif

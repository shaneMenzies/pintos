#ifndef CHUNKING_H
#define CHUNKING_H

#include "addressing.h"
#include "libk/callable.h"
#include "libk/functional.h"
#include "libk/mutex.h"
#include "memory/chunking_predef.h"
#include "memory/p_memory.h"
#include "memory/paging.h"
#include "system/error.h"
#include "system/pintos_std.h"
#include "threading/process_def.h"
#include "threading/threading.h"
#include "threading/topology.h"

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

  private:
    std_k::mutex access_lock;
    bool         can_free_reservoir;

  public:
    chunk_reservoir()
        : pile_index(pile_index)
        , chunk_size(chunk_size)
        , start(start)
        , last(last)
        , end(end)
        , access_lock()
        , can_free_reservoir(can_free_reservoir) {}
    chunk_reservoir(unsigned int pile_index)
        : pile_index(pile_index)
        , can_free_reservoir(false) {
        chunk_size = (PAGE_SIZE << (pile_index * 4));

        start = &reservoir_chunks[pile_index][1];
        last  = &reservoir_chunks[pile_index][0];
        end   = &reservoir_chunks[pile_index + 1][0];
    }

    int size() { return (last - start) + 1; }

    chunk get_chunk(bool lock_override = false);

    void get_chunks(int num_chunks, chunk* target_buffer,
                    bool lock_override = false) {

        if (!lock_override) access_lock.lock();

        for (int i = 0; i < num_chunks; i++) {
            target_buffer[i] = get_chunk(true);
        }

        if (!lock_override) access_lock.unlock();

        return;
    }

    void add_chunk(chunk new_chunk, bool lock_override = false) {

        if (new_chunk.size == 0) { return; }

        if (!lock_override) { access_lock.lock(); }

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

        if (!lock_override) { access_lock.unlock(); }
    }

    void add_chunks(chunk* new_chunks, int num_chunks,
                    bool lock_override = false) {

        if (!lock_override) access_lock.lock();

        for (int i = 0; i < num_chunks; i++) { add_chunk(new_chunks[i], true); }

        if (!lock_override) access_lock.unlock();
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

  private:
    chunk                                     chunks[CHUNKS_PER_PILE];
    int                                       next_chunk   = -1;
    threading::process*                       refresh_task = 0;
    std_k::mutex                              refresh_task_active;
    std_k::preset_function<void(chunk_pile&)> refresh_task_call;

    std_k::mutex access_lock;

  public:
    chunk_pile()
        : refresh_task_call(refill_chunks, *this) {};
    chunk_pile(unsigned int pile_index)
        : pile_index(pile_index)
        , refresh_task_call(refill_chunks, *this) {

        // Set chunk size
        chunk_size = (PAGE_SIZE << (pile_index * 4));

        // Fill chunks from reservoir
        for (unsigned int i = 0; i < CHUNKS_PER_PILE; i++) {
            chunks[i] = memory_reservoirs[pile_index].get_chunk();
            if (chunks[i].size) { next_chunk = i; }
        }

        // Create refill process
        refresh_task = new threading::process(4, 1, &refresh_task_call);
        refresh_task->config.wait_on_end = true;
    }

    int size() { return (next_chunk + 1); }

    static void refill_chunks(chunk_pile& target) {

        target.access_lock.lock();

        while (target.next_chunk < (int)(CHUNKS_PER_PILE - 1)) {
            target.next_chunk++;
            target.chunks[target.next_chunk]
                = memory_reservoirs[target.pile_index].get_chunk();
            if (target.chunks[target.next_chunk].size == 0) {
                target.next_chunk--;
                break;
            }
        }

        target.refresh_task_active.unlock();
        target.access_lock.unlock();
    }

    chunk get_chunk(bool pile_lock_override      = false,
                    bool reservoir_lock_override = false) {

        if (!pile_lock_override) { access_lock.lock(); }

        // Get the next chunk
        chunk value;
        if (next_chunk > -1) {
            // Try to grab from this pile
            value = chunks[next_chunk];
            next_chunk--;

            if (!pile_lock_override) { access_lock.unlock(); }
        } else {
            if (!pile_lock_override) { access_lock.unlock(); }

            // Try to grab from reservoir
            value = memory_reservoirs[pile_index].get_chunk(
                reservoir_lock_override);
        }

        // Send a thread to refill chunks
        if (!pile_lock_override && next_chunk < (int)(CHUNKS_PER_PILE / 4)
            && !refresh_task_active.is_locked()) {
            if (refresh_task_active.try_lock()) {
                refresh_task->rounds = 1;
                threading::system_scheduler.add_process(refresh_task);
            }
        }

        return value;
    }

    void get_chunks(int num_chunks, chunk* target_buffer,
                    bool pile_lock_override      = false,
                    bool reservoir_lock_override = false) {

        if (!pile_lock_override) access_lock.lock();

        for (int i = 0; i < num_chunks; i++) {
            target_buffer[i] = get_chunk(true, reservoir_lock_override);
        }

        // Send a thread to refill chunks
        if (!pile_lock_override && next_chunk < (int)(CHUNKS_PER_PILE / 4)
            && !refresh_task_active.is_locked()) {
            if (refresh_task_active.try_lock()) {
                refresh_task->rounds = 1;
                threading::system_scheduler.add_process(refresh_task);
            }
        }

        if (!pile_lock_override) access_lock.unlock();

        return;
    }
};

void turn_to_chunks(uintptr_t start_address, size_t size);
void construct_chunks(multiboot_boot_info* mb_info);
void mmap_to_chunks(multiboot_mmap_entry target);
} // namespace chunking

#pragma GCC diagnostic pop

#endif

/**
 * @file super_memory.cpp
 * @author Shane Menzies
 * @brief Super-page memory management
 * @date 01/03/22
 *
 *
 */

#include "super_memory.h"

#include "addressing.h"
#include "allocation_manager.h"
#include "libk/misc.h"
#include "threading/threading.h"

allocation_entry* chunk_alloc(size_t size) {
    unsigned int required_chunks = chunking::get_num_chunks(size, true);

    size_t added_size = (sizeof(chunking::chunk) * required_chunks)
                        + sizeof(allocation_entry);
    size += added_size;

    chunking::chunk* chunk_location
        = (chunking::chunk*)paging::kernel_address_space.get_new_address(size);
    allocation_entry* entry
        = (allocation_entry*)((uintptr_t)chunk_location
                              + (sizeof(chunking::chunk) * required_chunks));
    void* mapped_location
        = (void*)((uintptr_t)entry + sizeof(allocation_entry));
    void* next_to_map = chunk_location;

    chunking::chunk_pile* current_piles = current_thread()->memory_piles;

    // Get all the required chunks
    uint64_t current_size = PAGE_SIZE << ((chunking::NUM_MEMORY_PILES - 1) * 4);
    unsigned int current_index     = (chunking::NUM_MEMORY_PILES - 1);
    unsigned int saved_chunk_index = 0;

    chunking::chunk temp_buffer[0x10];
    while (current_size >= PAGE_SIZE) {

        unsigned int num_chunks = size / current_size;
        size %= current_size;

        if (!current_index && size) {
            num_chunks++;
            size = 0;
        }

        if (num_chunks > 0) {
            // Get the needed chunks of this size
            current_piles[current_index].get_chunks(num_chunks, temp_buffer);

            // Save them in memory
            for (unsigned int i = 0; i < num_chunks; i++) {
                next_to_map = temp_buffer[i].map_to(next_to_map);
                chunk_location[saved_chunk_index] = temp_buffer[i];
                saved_chunk_index++;
            }
        }

        if (size == 0) { break; }

        current_size >>= 4;
        current_index--;
    }

    entry->value.address    = (uintptr_t)mapped_location;
    entry->value.chunks     = chunk_location;
    entry->value.num_chunks = required_chunks;

    return entry;
}

allocation_entry* aligned_chunk_alloc(size_t size, size_t alignment) {
    unsigned int required_chunks = chunking::get_num_chunks(size, true);

    size_t added_size = (sizeof(chunking::chunk) * required_chunks)
                        + sizeof(allocation_entry);
    added_size = std_k::round_up_multiple(added_size, alignment);
    size += added_size;

    chunking::chunk* chunk_location
        = (chunking::chunk*)paging::kernel_address_space.get_new_address(size);
    allocation_entry* entry
        = (allocation_entry*)((uintptr_t)chunk_location
                              + (sizeof(chunking::chunk) * required_chunks));
    void* mapped_location = (void*)((uintptr_t)entry + added_size);
    void* next_to_map     = chunk_location;

    chunking::chunk_pile* current_piles = current_thread()->memory_piles;

    // Get all the required chunks
    uint64_t current_size = PAGE_SIZE << ((chunking::NUM_MEMORY_PILES - 1) * 4);
    unsigned int current_index     = (chunking::NUM_MEMORY_PILES - 1);
    unsigned int saved_chunk_index = 0;

    chunking::chunk temp_buffer[0x10];
    while (current_size >= PAGE_SIZE) {

        unsigned int num_chunks = size / current_size;
        size %= current_size;

        if (!current_index && size) {
            num_chunks++;
            size = 0;
        }

        if (num_chunks > 0) {
            // Get the needed chunks of this size
            current_piles[current_index].get_chunks(num_chunks, temp_buffer);

            // Save them in memory
            for (unsigned int i = 0; i < num_chunks; i++) {
                next_to_map = temp_buffer[i].map_to(next_to_map);
                chunk_location[saved_chunk_index] = temp_buffer[i];
                saved_chunk_index++;
            }
        }

        if (size == 0) { break; }

        current_size >>= 4;
        current_index--;
    }

    entry->value.address    = (uintptr_t)mapped_location;
    entry->value.chunks     = chunk_location;
    entry->value.num_chunks = required_chunks;

    return entry;
}

allocation_entry* bootstrap_chunk_alloc(size_t size) {
    unsigned int required_chunks = chunking::get_num_chunks(size, true);

    size_t added_size = (sizeof(chunking::chunk) * required_chunks)
                        + sizeof(allocation_entry);
    size += added_size;

    chunking::chunk* chunk_location
        = (chunking::chunk*)paging::kernel_address_space.get_new_address(size);
    allocation_entry* entry
        = (allocation_entry*)((uintptr_t)chunk_location
                              + (sizeof(chunking::chunk) * required_chunks));
    void* mapped_location
        = (void*)((uintptr_t)entry + sizeof(allocation_entry));
    void* next_to_map = chunk_location;

    // Get all the required chunks
    uint64_t current_size = PAGE_SIZE << ((chunking::NUM_MEMORY_PILES - 1) * 4);
    unsigned int current_index     = (chunking::NUM_MEMORY_PILES - 1);
    unsigned int saved_chunk_index = 0;

    chunking::chunk temp_buffer[0x10];
    while (current_size >= PAGE_SIZE) {

        unsigned int num_chunks = size / current_size;
        size %= current_size;

        if (!current_index && size) {
            num_chunks++;
            size = 0;
        }

        if (num_chunks > 0) {
            // Get the needed chunks of this size
            chunking::memory_reservoirs[current_index].get_chunks(num_chunks,
                                                                  temp_buffer);

            // Map the chunks and save them in memory
            for (unsigned int i = 0; i < num_chunks; i++) {
                next_to_map = temp_buffer[i].map_to(next_to_map);
                chunk_location[saved_chunk_index] = temp_buffer[i];
                saved_chunk_index++;
            }
        }

        if (size == 0) { break; }

        current_size >>= 4;
        current_index--;
    }

    entry->value.address    = (uintptr_t)mapped_location;
    entry->value.chunks     = chunk_location;
    entry->value.num_chunks = required_chunks;

    // Make a new allocation entry
    return entry;
}

allocation_entry* bootstrap_aligned_chunk_alloc(size_t size, size_t alignment) {
    unsigned int required_chunks = chunking::get_num_chunks(size, true);

    size_t added_size = (sizeof(chunking::chunk) * required_chunks)
                        + sizeof(allocation_entry);
    added_size = std_k::round_up_multiple(added_size, alignment);
    size += added_size;

    chunking::chunk* chunk_location
        = (chunking::chunk*)paging::kernel_address_space.get_new_address(size);
    allocation_entry* entry
        = (allocation_entry*)((uintptr_t)chunk_location
                              + (sizeof(chunking::chunk) * required_chunks));
    void* mapped_location = (void*)((uintptr_t)entry + added_size);
    void* next_to_map     = chunk_location;

    // Get all the required chunks
    uint64_t current_size = PAGE_SIZE << ((chunking::NUM_MEMORY_PILES - 1) * 4);
    unsigned int current_index     = (chunking::NUM_MEMORY_PILES - 1);
    unsigned int saved_chunk_index = 0;

    chunking::chunk temp_buffer[0x10];
    while (current_size >= PAGE_SIZE) {

        unsigned int num_chunks = size / current_size;
        size %= current_size;

        if (!current_index && size) {
            num_chunks++;
            size = 0;
        }

        if (num_chunks > 0) {
            // Get the needed chunks of this size
            chunking::memory_reservoirs[current_index].get_chunks(num_chunks,
                                                                  temp_buffer);

            // Map the chunks and save them in memory
            for (unsigned int i = 0; i < num_chunks; i++) {
                next_to_map = temp_buffer[i].map_to(next_to_map);
                chunk_location[saved_chunk_index] = temp_buffer[i];
                saved_chunk_index++;
            }
        }

        if (size == 0) { break; }

        current_size >>= 4;
        current_index--;
    }

    entry->value.address    = (uintptr_t)mapped_location;
    entry->value.chunks     = chunk_location;
    entry->value.num_chunks = required_chunks;

    // Make a new allocation entry
    return entry;
}

allocation_entry* override_chunk_alloc(size_t size, int lock_override_index) {
    unsigned int required_chunks = chunking::get_num_chunks(size, true);

    size_t added_size = (sizeof(chunking::chunk) * required_chunks)
                        + sizeof(allocation_entry);
    size += added_size;

    chunking::chunk* chunk_location
        = (chunking::chunk*)paging::kernel_address_space.get_new_address(size);
    allocation_entry* entry
        = (allocation_entry*)((uintptr_t)chunk_location
                              + (sizeof(chunking::chunk) * required_chunks));
    void* mapped_location
        = (void*)((uintptr_t)entry + sizeof(allocation_entry));
    void* next_to_map = chunk_location;

    // Get all the required chunks
    uint64_t current_size = PAGE_SIZE << ((chunking::NUM_MEMORY_PILES - 1) * 4);
    unsigned int current_index     = (chunking::NUM_MEMORY_PILES - 1);
    unsigned int saved_chunk_index = 0;

    chunking::chunk temp_buffer[0x10];
    while (current_size >= PAGE_SIZE) {

        unsigned int num_chunks = size / current_size;
        size %= current_size;

        if (!current_index && size) {
            num_chunks++;
            size = 0;
        }

        if (num_chunks > 0) {
            // Get the needed chunks of this size
            chunking::memory_reservoirs[current_index].get_chunks(
                num_chunks, temp_buffer,
                ((int)current_index == lock_override_index) ? true : false);

            // Map the chunks and save them in memory
            for (unsigned int i = 0; i < num_chunks; i++) {
                next_to_map = temp_buffer[i].lock_override_map_to(
                    next_to_map, lock_override_index);
                chunk_location[saved_chunk_index] = temp_buffer[i];
                saved_chunk_index++;
            }
        }

        if (size == 0) { break; }

        current_size >>= 4;
        current_index--;
    }

    entry->value.address    = (uintptr_t)mapped_location;
    entry->value.chunks     = chunk_location;
    entry->value.num_chunks = required_chunks;

    // Make a new allocation entry
    return entry;
}

void chunk_free(allocation_info returned_entry) {
    chunking::chunk* returned_chunks = returned_entry.chunks;
    unsigned int     num_chunks      = returned_entry.num_chunks;

    for (unsigned int i = 0; i < num_chunks; i++) {
        chunking::chunk next_chunk = returned_chunks[i];

        chunking::memory_reservoirs[chunking::get_chunk_index(next_chunk.size)]
            .add_chunk(next_chunk);
    }
}

void override_chunk_free(allocation_info returned_entry,
                         int             lock_override_index) {
    chunking::chunk* returned_chunks = returned_entry.chunks;
    unsigned int     num_chunks      = returned_entry.num_chunks;

    for (unsigned int i = 0; i < num_chunks; i++) {
        chunking::chunk next_chunk = returned_chunks[i];
        int target_index           = chunking::get_chunk_index(next_chunk.size);

        chunking::memory_reservoirs[target_index].add_chunk(
            next_chunk, (target_index == lock_override_index) ? true : false);
    }
}

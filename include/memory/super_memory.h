#ifndef P_SUPER_MEMORY_H
#define P_SUPER_MEMORY_H

#include "memory/p_memory.h"
#include "stddef.h"
#include "stdint.h"

class allocation_manager;
struct allocation_info;
struct allocation_entry;

allocation_entry* chunk_alloc(size_t size);
allocation_entry* aligned_chunk_alloc(size_t size, size_t alignment);

allocation_entry* bootstrap_chunk_alloc(size_t size);
allocation_entry* bootstrap_aligned_chunk_alloc(size_t size, size_t alignment);

allocation_entry* override_chunk_alloc(size_t size,
                                       int    lock_override_index = -1);

void chunk_free(allocation_info returned_entry);
void override_chunk_free(allocation_info returned_entry,
                         int             lock_override_index = -1);

#endif
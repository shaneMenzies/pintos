#ifndef CHUNKING_PREDEF_H
#define CHUNKING_PREDEF_H

#include "memory/p_memory.h"

namespace chunking {

struct chunk_pile;
struct chunk_reservoir;
struct chunk;

constexpr unsigned int NUM_MEMORY_PILES       = 6;
constexpr unsigned int CHUNKS_PER_PILE        = 0x20;
constexpr unsigned int RESERVOIR_DEFAULT_SIZE = 128;

extern chunk reservoir_chunks[NUM_MEMORY_PILES][RESERVOIR_DEFAULT_SIZE];
extern chunk_reservoir memory_reservoirs[NUM_MEMORY_PILES];

#define get_chunk_size(index) PAGE_SIZE << (index * 4)

} // namespace chunking

#endif
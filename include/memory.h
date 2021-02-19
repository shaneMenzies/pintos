#ifndef MEMORY_H
#define MEMORY_H

#include "multiboot.h"

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Each block has a corresponding header type, which line up 
 *        in a way where the header could be considered to almost be 
 *        the 1025th piece of the next step up.
 * 
 *        i.e. 1 gigablock would need 1 megaheader and 1024 megablocks
 * 
 *        This system has been designed around systems with memory in
 *        the gigabytes, however it should be fairly adaptable without
 *        too much difficulty, just by modifying the makeup of the master
 *        header
 * 
 */
/*
struct kiloheader {
    uint16_t sp[1024];
};

typedef struct kiloblock {
    unsigned char byte[1024]; 
} kiloblock;

struct megaheader {
    struct kiloheader kilo;
    uint8_t header[1024];
    // Takes up 3 kiloblocks
};

typedef struct megablock {
    kiloblock kilo[1024];
} megablock;

struct gigaheader {
    struct megaheader mega[1024];
    uint8_t header[1024]; 
    // takes up 3097 kiloblocks
};

typedef struct gigablock {
    megablock mega[1024];
} gigablock;



struct teraheader {
    struct gigaheader giga[1024];
    uint8_t header[1024];
};

// Terabyte memory not currently Implemented
typedef struct terablock {
    gigablock at[1024];
} terablock;

*/

enum memory_constants {
    KILOBLOCK_SIZE = 1024,
    MEGABLOCK_SIZE = KILOBLOCK_SIZE * 1024,
    GIGABLOCK_SIZE = MEGABLOCK_SIZE * 1024,
};

void memory_init(struct mb_info* mb_addr);

void* malloc(size_t size);

#endif
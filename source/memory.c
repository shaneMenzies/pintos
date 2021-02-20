/**
 * @file memory.c
 * @author Shane Menzies
 * @brief Functions for memory management
 * @date 02/14/21
 * 
 * 
 */

#include "memory.h"

#include "multiboot.h"

#include <stddef.h>
#include <stdint.h>

bookmark kernel_mark[256];

/**
 * @brief Converts a mmap size/info structure into a bookmark
 * 
 * @param mmap_addr     Pointer to mmap size/info structure
 * @return bookmark     Bookmark filled with info from mmap structure
 */
bookmark mmap_to_mark (uint32_t* mmap_addr) {

    void* start = (void*)(*(void**)(mmap_addr + 1));
    void* end = (void*)(start + (unsigned int)(*(void**)(mmap_addr + 3)));
    uint32_t flags = *(mmap_addr + 5);

    bookmark return_mark;
    return_mark.start = start;
    return_mark.end = end;
    if (flags == 1) {
        return_mark.available_flag = true;
    } else {
        return_mark.available_flag = false;
    }

    return return_mark;
}

/**
 * @brief Initializes the kernel bookmarks from a multiboot structure
 * 
 * @param mb_addr   Pointer to a mb_info structure
 */
void memory_init(struct mb_info* mb_addr) {

    uint32_t mmap_length = mb_addr->mmap_length;
    unsigned char* mmap_index = (unsigned char*)mb_addr->mmap_addr;
    unsigned char* mmap_end = (unsigned char*)(mmap_index + mmap_length);

    uint16_t kernel_index = 0;
    
    while (mmap_index < mmap_end) {
        uint32_t size = *(uint32_t*)mmap_index;
        kernel_mark[kernel_index] = mmap_to_mark((uint32_t*)mmap_index);
        mmap_index += size;
    }

    // Finish kernel marks with a null terminator
    kernel_mark[kernel_index].start = 0;
    kernel_mark[kernel_index].end = 0;
    kernel_mark[kernel_index].available_flag = 0;
}

/**
 * @brief Allocates an area of memory the size of size in bytes
 * 
 * @param size      Bytes to allocate
 * @return void*    Pointer to start of newly allocated memory
 */
void* malloc(size_t size) {

    uint32_t index = 0;

    while (1) {
        bookmark target_mark = kernel_mark[index];

        // Check for null mark
        if (target_mark.start == 0 && target_mark.end == 0) {
            //TODO: ERROR: No memory area available
            return (void*)(0);
        }

        // If the mark isn't available, just skip past it
        if (!target_mark.available_flag) {
            index++;
            continue;
        }

        unsigned int mark_size = (unsigned int)(target_mark.end 
                                                - target_mark.start);
        if (mark_size >= size) {
            // This mark is big enough

            // Split target mark into two
            bookmark new_mark;
            new_mark.start = target_mark.start;
            new_mark.end = (void*)(target_mark.start + size);
            new_mark.available_flag = false;

            target_mark.start = new_mark.end + 1;
            // If the target mark has been used up entirely, just replace it
            if (target_mark.end >= target_mark.start) {
                kernel_mark[index] = new_mark;
                return new_mark.start;
            } else {

                // Shift all marks below new one
                for (uint32_t reverse_index = (256 - index); 
                     reverse_index >= index; reverse_index--) {
                        kernel_mark[reverse_index] = kernel_mark[reverse_index - 1];
                }

                // Insert new mark
                kernel_mark[index] = new_mark;
                return new_mark.start;
            }
        } else {
            // This mark is not suitable
            index++;
        }
    }
}

/**
 * @brief Allocates the area of memory at a certain address
 * 
 * @param target_address    Address to start allocation at
 * @param size              Size of memory to allocate
 * @return void*            Pointer to start of newly allocated memory
 */
void* talloc(void* target_address, size_t size) {

    uint32_t index = 0;

    // Find mark to split up
    while (1) {
        if (kernel_mark[index].start <= target_address) {
            break;
        }
        index++;
    }

    bookmark target_mark = kernel_mark[index];

    // Make new mark
    bookmark new_mark;
    new_mark.start = target_address;
    new_mark.end = (void*) (target_address + size);
    new_mark.available_flag = false;

    // Correct the target mark being split
    target_mark.end = (target_address - 1);

    // Place new mark
    if (!(target_mark.end >= target_mark.start)) {
        // If we aren't replacing this mark, then
        // shift all marks forward
        for (uint32_t reverse_index = (256 - index); 
             reverse_index >= index; reverse_index--) {
                kernel_mark[reverse_index] = kernel_mark[reverse_index - 1];
        }
    }

    kernel_mark[index] = new_mark;

    // Check if the marks in front have overlap
    while (1) {

        target_mark = kernel_mark[++index];
        // If the mark is at all inside the new one, and isn't allocated,
        // then fix it's properties
        if (target_mark.start < new_mark.end && target_mark.available_flag) {
            target_mark.start = (void*)(new_mark.end + 1);

            // If this mark is entirely encapsulated by new one, remove it
            if (target_mark.end <= target_mark.start) {
                for (uint32_t forward_index = index; forward_index < 256; 
                    forward_index++) {
                        kernel_mark[forward_index] = kernel_mark[forward_index + 1];
                }
            }
        } else if (target_mark.start < new_mark.end) {
            continue;
        } else {
            break;
        }
    }

    return new_mark.start;
}

/**
 * @brief Frees the area of memory starting at the provided address
 * 
 * @param target_address    Address of the start of the area to be freed
 */
void free(void* target_address) {

    uint32_t index = 0;
    bookmark target_mark;

    // Find the mark associated with the address
    while (1) {

        if (index >= 256) {
            //TODO: ERROR: ASSOCIATED MARK NOT FOUND
            return;
        }

        target_mark = kernel_mark[index];
        if (target_mark.start == target_address) {
            break;
        }

        index++;
    }

    // TODO: MERGE NEARBY FREE MARKS
    target_mark.available_flag = true;
}
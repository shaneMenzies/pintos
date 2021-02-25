/**
 * @file memory.c
 * @author Shane Menzies
 * @brief Functions for memory management
 * @date 02/14/21
 * 
 * 
 */

#include "memory.h"

#include "kernel.h"
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
 * @brief Finds the position of the null marker in kernel_mark
 * 
 * @param start     Place to start search
 * @return int      Position of first null marker
 */
int find_null_mark(int start) {

    int index = start;
    while (1) {
        
        if (kernel_mark[index].start == 0 
            && kernel_mark[index].end == 0) {
                break;
        } else {
            index++;
        }
    }

    return index;
}

/**
 * @brief Cycles the kernel_marks up, leaving a space the size of count
 *        at index start in kernel_mark
 * 
 * @param start     Index in kernel_mark to start cycling up
 * @param count     Times to cycle the target marks up
 */
void cycle_marks_up(int start, int count) {

    // Find end of marks
    int index = find_null_mark(start);

    // Shift the marks up
    for (; index >= start; index--) {

            kernel_mark[index] = kernel_mark[index - count];
            if (index == start) {
                break;
            }
    }
}

/**
 * @brief Cycles the marks from start to the end of the marks down count times
 * 
 * @param start     Index to start cycling down at 
 *                  (note: the mark here will be erased, and replaced by
 *                   the one count times in front of it)
 * @param count     Marks to cycle down
 */
void cycle_marks_down(int start, int count) {

    // Find where the null marker will end up, in order to stop there
    int end = find_null_mark(start) - count;

    int index = start;

    // Shift the marks down
    for (; index <= end; index++) {
        kernel_mark[index] = kernel_mark[index + count];
    }
}

/**
 * @brief Initializes the kernel bookmarks from a multiboot structure
 * 
 * @param mb_addr   Pointer to a mb_info structure
 */
void memory_init(struct mb_info* mb_addr) {

    uint32_t mmap_length = mb_addr->mmap_length;
    unsigned char* mmap_index = (unsigned char*)(mb_addr->mmap_addr);
    unsigned char* mmap_end = (unsigned char*)(mmap_index + mmap_length);

    uint16_t kernel_index = 0;
    
    while (mmap_index < mmap_end) {
        uint32_t size = (*(uint32_t*)mmap_index + 4);
        kernel_mark[kernel_index] = mmap_to_mark((uint32_t*)mmap_index);
        mmap_index = (unsigned char*) ((unsigned int)mmap_index + size);
        kernel_index++;
    }

    // Finish kernel marks with a null terminator
    kernel_mark[kernel_index].start = 0;
    kernel_mark[kernel_index].end = 0;
    kernel_mark[kernel_index].available_flag = 0;

    // Allocate first 4 bytes of memory for error codes
    talloc(0, 4);
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
            if (target_mark.end <= target_mark.start) {
                kernel_mark[index] = new_mark;
                return new_mark.start;
            } else {

                // Save changes made to target mark
                kernel_mark[index] = target_mark;

                // Cycle the marks up 1
                cycle_marks_up(index, 1);                

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
        if (kernel_mark[index].start <= target_address 
            && kernel_mark[index].end >= target_address) {
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

    // Check if the target mark still needs to exist
    if (target_mark.end >= target_mark.start) {
        // Target mark does need to be placed back
        kernel_mark[index] = target_mark;

        // If we aren't replacing this mark, then
        // shift all marks forward
        cycle_marks_up(index, 1);
    }

    // Place new mark
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
                
                cycle_marks_down(index, 1);
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

    target_mark.available_flag = true;

    // Merge nearby free marks
    if (kernel_mark[index + 1].available_flag) {
        // Merge mark in front
        target_mark.end = kernel_mark[index + 1].end;
        cycle_marks_down((index + 1), 1);
    }

    if (kernel_mark[index - 1].available_flag) {
        target_mark.start = kernel_mark[index - 1].start;
        cycle_marks_down((index - 1), 1);
    }

    kernel_mark[index] = target_mark;
}
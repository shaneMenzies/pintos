/**
 * @file memory.cpp
 * @author Shane Menzies
 * @brief Functions for memory management
 * @date 02/14/21
 * 
 * 
 */

#include "memory.h"

trees::mark_tree free_marks;
trees::mark_tree used_marks;

trees::bookmark bootstrap_bank[256];

void* mkalloc_start;
void* mkalloc_end;

void* max_phys_address = 0;

void* operator new(size_t size) {
    return malloc(size);
}
 
void* operator new[](size_t size) {
    return malloc(size);
}
 
void operator delete(void* p) {
    free(p);
}
 
void operator delete[](void* p) {
    free(p);
}

void operator delete(void* p, long unsigned int size) {
    (void) size;
    free(p);
}

void operator delete[](void* p, long unsigned int size) {
    (void) size;
    free(p);
}

/**
 * @brief Allocates space for a new bookmark, and returns a pointer
 *        to the newly allocated mark
 * 
 * @return trees::bookmark*     Pointer to new mark
 */
trees::bookmark* mkalloc() {

    // Flag for if more space is currently being allocated
    static bool mkalloc_active = false;

    // Put new bookmark at current position of mkalloc_start
    trees::bookmark* new_mark = (trees::bookmark*) mkalloc_start;
    new_mark->balance = 0;
    new_mark->flags = 0;

    // Increase mkalloc_start
    mkalloc_start = (void*)((uintptr_t)mkalloc_start + sizeof(trees::bookmark));

    // Check if more space needs to be allocated
    if (((uintptr_t)mkalloc_end - (uintptr_t)mkalloc_start) <= (4 * sizeof(trees::bookmark)) 
         && !mkalloc_active) {

        // Now allocating more space
        mkalloc_active = true;
        mkalloc_start = malloc(128 * sizeof(trees::bookmark));
        mkalloc_end = (void*)((uintptr_t)mkalloc_start + (128 * sizeof(trees::bookmark)));
        mkalloc_active = false;
    }

    // Return new mark
    return new_mark;
}

/**
 * @brief Converts a mmap size/info structure into a bookmark
 * 
 * @param mmap_addr     Pointer to mmap size/info structure
 * @return bookmark     Bookmark filled with info from mmap structure
 */
trees::bookmark mmap_to_mark (uint32_t* mmap_addr) {

    void* start = (void*)(*(uint64_t*)(mmap_addr + 1));
    void* end = (void*)((uintptr_t)start + (uintptr_t)(*(uint64_t*)(mmap_addr + 3)));
    uint32_t flags = *(mmap_addr + 5);

    trees::bookmark return_mark(start, end);

    if (flags == 1) {
        return_mark.flags |= trees::MARK_FREE;
    } else {
        return_mark.flags &= ~(trees::MARK_FREE);
    }

    return return_mark;
}

/**
 * @brief Initializes the kernel bookmarks from a multiboot structure
 * 
 * @param mb_addr   Pointer to a mb_info structure
 */
void memory_init(struct mb_info* mb_addr) {

    // Start up mark allocation on bootstrap bank
    mkalloc_start = (void*)bootstrap_bank;
    mkalloc_end = (void*)((uintptr_t)mkalloc_start + (256 * sizeof(trees::bookmark)));

    // Prepare the memory trees
    free_marks.size_sorted = true;
    used_marks.size_sorted = false;

    uint32_t mmap_length = mb_addr->mmap_length;
    unsigned char* mmap_index = (unsigned char*)(mb_addr->mmap_addr);
    unsigned char* mmap_end = (unsigned char*)(mmap_index + mmap_length);
    
    // Loop through the mmap entries, adding them to the trees
    while (mmap_index < mmap_end) {
        uint32_t size = (*(uint32_t*)mmap_index + 4);
        trees::bookmark* converted_mark = mkalloc();
        *converted_mark = mmap_to_mark((uint32_t*)mmap_index);

        if (converted_mark->flags & trees::MARK_FREE) {
            // Check if this is a new greatest accessible address
            if (converted_mark->end > max_phys_address)
                max_phys_address = converted_mark->end;

            free_marks.insert(converted_mark);
        } else {
            used_marks.insert(converted_mark);
        }

        mmap_index = (unsigned char*) ((uintptr_t)mmap_index + size);
    }
}

/**
 * @brief Allocates an area of memory the size of size in bytes
 * 
 * @param size      Bytes to allocate
 * @return void*    Pointer to start of newly allocated memory
 */
void* malloc(size_t size) {

    // Find a suitable mark, and remove it from the free marks
    trees::bookmark* target_mark = free_marks.find_suitable(size);
    free_marks.seperate(target_mark);

    // Save the address to return, and prepare a new mark
    void* return_address = target_mark->start;
    trees::bookmark* new_mark;

    // Determine if the mark should be put back or not
    if (target_mark->size > size) {

        // Needs to be put back, and a new mark is needed
        target_mark->start = (void*)((uintptr_t)target_mark->start + size);
        target_mark->size = (size_t)((uintptr_t)target_mark->end 
                                     - (uintptr_t)target_mark->start);
        target_mark->flags = 0;
        target_mark->balance = 0;
        free_marks.insert(target_mark);

        new_mark = mkalloc();
        new_mark->start = return_address;

    } else {

        // Don't need to put the mark back, so it can just be reused
        new_mark = target_mark;
    }

    // Prepare, and insert, the new mark
    new_mark->size = size;
    new_mark->end = (void*)((uintptr_t)return_address + size);
    new_mark->flags &= ~(trees::MARK_FREE);
    used_marks.insert(new_mark);

    return return_address;
}

/**
 * @brief Allocates the area of memory at a certain address. 
 * 
 * @param target_address    Address to start allocation at
 * @param size              Size of memory to allocate
 * @return void*            Pointer to start of newly allocated memory
 */
void* talloc(void* target_address, size_t size) {

    void* end_address = (void*)((uintptr_t)target_address + size);

    // Find mark to split up
    trees::bookmark* first_mark = free_marks.find_containing(target_address);

    trees::bookmark* target_mark = first_mark;

    trees::bookmark* last_mark;
    bool have_last_mark = false;

    // Do any other marks need split up too?
    if (target_mark->end < end_address) {

        while (target_mark->end < end_address) {
            target_mark = free_marks.find((void*)((uintptr_t)target_mark->end + 1));
            free_marks.seperate(target_mark);
        }
        last_mark = target_mark;
        have_last_mark = true;
    }
    
    free_marks.seperate(first_mark);

    trees::bookmark* new_mark = 0;

    // Split the old marks
    if (!have_last_mark) {

        // If we cut space out of just the start, we don't need a new mark
        if (first_mark->start == target_address) {
            first_mark->start = (void*)((uintptr_t)end_address + 1);
            first_mark->size = first_mark->size - size;
            first_mark->balance = 0;
            first_mark->flags = 0;
            free_marks.insert(first_mark);
        } else {
            // Get a new mark
            new_mark = mkalloc();

            // Correct these marks
            new_mark->start = first_mark->start;
            new_mark->end = (void*)((uintptr_t)target_address - 1);
            new_mark->size = (size_t)((uintptr_t)new_mark->end - (uintptr_t)new_mark->start);
            first_mark->start = (void*)((uintptr_t)end_address + 1);
            first_mark->size = (size_t)((uintptr_t)first_mark->end - (uintptr_t)first_mark->start);
            first_mark->balance = 0;
            first_mark->flags = 0;

            // Insert them
            free_marks.insert(first_mark);
            free_marks.insert(new_mark);
        }
    } else {

        // First mark has had it's back cut off
        first_mark->end = (void*)((uintptr_t)target_address - 1);
        first_mark->size = (size_t)((uintptr_t)first_mark->end - (uintptr_t)first_mark->start);
        first_mark->balance = 0;
        first_mark->flags = 0;

        // Last mark has had it's front cut off
        last_mark->start = (void*)((uintptr_t)end_address + 1);
        last_mark->size = (size_t)((uintptr_t)last_mark->end - (uintptr_t)last_mark->start);
        last_mark->balance = 0;
        last_mark->flags = 0;

        // Put them back
        free_marks.insert(first_mark);
        free_marks.insert(last_mark);
    }
    
    // Prepare, and insert, a new used space mark
    new_mark = mkalloc();
    new_mark->start = target_address;
    new_mark->size = size;
    new_mark->end = end_address;
    used_marks.insert(new_mark);

    return target_address;
}

/**
 * @brief Frees the area of memory starting at the provided address
 * 
 * @param target_address    Address of the start of the area to be freed
 */
void free(void* target_address) {

    // Find the right mark
    trees::bookmark* target_mark = used_marks.find(target_address);

    // Remove the mark
    used_marks.seperate(target_mark);
}

// TODO: MEMORY CLEANUP

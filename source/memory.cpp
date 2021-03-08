/**
 * @file memory.c
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

    void* start = (void*)(*(void**)(mmap_addr + 1));
    void* end = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(start) + reinterpret_cast<uintptr_t>(*((void**)(mmap_addr + 3))));
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
            free_marks.insert(converted_mark);
        } else {
            used_marks.insert(converted_mark);
        }

        mmap_index = (unsigned char*) ((uintptr_t)mmap_index + size);
    }

    // Allocate first few bytes of memory for error codes
    talloc(error_code_addr, (sizeof(uint32_t) + sizeof(char*)));
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
        new_mark->balance = 0;
        new_mark->flags = 0;
    }

    // Prepare, and insert, the new mark
    new_mark->size = size;
    new_mark->end = (void*)((uintptr_t)return_address + size);
    new_mark->flags &= ~(trees::MARK_FREE);
    used_marks.insert(new_mark);

    return return_address;
}

/**
 * @brief Allocates the area of memory at a certain address. Requires 
 *        there to exist a certain mark with a start address value
 *        at the requested address. Often will result in strange
 *        and/or inconsistent results if used outside of certain very
 *        specific cases.
 * 
 * @param target_address    Address to start allocation at
 * @param size              Size of memory to allocate
 * @return void*            Pointer to start of newly allocated memory
 */
void* talloc(void* target_address, size_t size) {

    // Find mark to split up
    trees::bookmark* target_mark = free_marks.find(target_address);
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
        new_mark->balance = 0;
        new_mark->flags = 0;
    }

    // Prepare, and insert, the new mark
    new_mark->size = size;
    new_mark->end = (void*)((uintptr_t)return_address + size);
    used_marks.insert(new_mark);

    return return_address;
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

/**
 * @file memory.cpp
 * @author Shane Menzies
 * @brief Functions for memory management
 * @date 02/14/21
 * 
 * 
 */

#include "p_memory.h"

extern "C" {
    extern uintptr_t kernel_start;
    extern uintptr_t kernel_end;
}

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

const unsigned int num_protected_regions = 6;
void* protected_regions[num_protected_regions][2] = {
    {(void*)&kernel_start, (void*)((uintptr_t)&kernel_end - (uintptr_t)&kernel_start)},
    {0, 0}, // Boot processor's stack
    {0, 0}, // Boot data 
    {0, 0}, // Multiboot info
    {0, 0}, // Additional thread startup code
    {0, (void*)32}
};

void fill_protected_regions(multiboot_boot_info* mb_info) {
    // Stack
    protected_regions[1][0] = mb_info->stack_bottom;
    protected_regions[1][1] = (void*)((uintptr_t)mb_info->stack_top - (uintptr_t)mb_info->stack_bottom);

    // Boot data
    protected_regions[2][0] = mb_info->boot_start;
    protected_regions[2][1] = (void*)mb_info->boot_size;

    // Multiboot info
    protected_regions[3][0] = mb_info->mb_start;
    protected_regions[3][1] = (void*)mb_info->mb_size;

    // Additional thread startup code
    protected_regions[4][0] = mb_info->thread_start;
    protected_regions[4][1] = (void*)mb_info->thread_size;
}

void clear_protected_regions(void* added_start, size_t added_size) {
    void* added_end = (void*)((uintptr_t)added_start + added_size);

    // Compare what's been added to each protected region
    for (unsigned int i = 0; i < num_protected_regions; i++) {
        void* protected_start = protected_regions[i][0];
        void* protected_end = (void*)((uintptr_t)protected_start + (uintptr_t)protected_regions[i][1]);
        bool in_protected = false;

        in_protected |= ((added_end >= protected_start) && (added_end <= protected_end));
        in_protected |= ((added_start <= protected_start) && (added_end >= protected_end));
        in_protected |= ((added_start >= protected_start) && (added_start <= protected_end));

        if (in_protected) {
            // Need to clear this protected region of memory
            talloc(protected_regions[i][0], (size_t)protected_regions[i][1]);
        }
    }
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
 * @brief Converts a multiboot mmap entry into a bookmark
 * 
 * @param mmap          Pointer to multiboot mmap entry structure
 * @return bookmark     Bookmark filled with info from mmap structure
 */
trees::bookmark mmap_to_mark (multiboot_mmap_entry* mmap) {

    void* start = (void*)mmap->addr;
    void* end = (void*)((uintptr_t)start + (uintptr_t)(mmap->len));

    trees::bookmark return_mark(start, end);

    // Check the mmap type
    switch (mmap->type) {

        case MULTIBOOT_MEMORY_AVAILABLE:
            return_mark.flags |= trees::MARK_FREE;
            break;

        case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
            /* fallthrough */
        default:
            return_mark.flags &= ~(trees::MARK_FREE); 
    }

    return return_mark;
}

/**
 * @brief Converts an efi mmap entry into a bookmark
 * 
 * @param mmap          Pointer to efi mmap entry structure
 * @return bookmark     Bookmark filled with info from mmap structure
 */
trees::bookmark mmap_to_mark (efi_mmap_entry* mmap) {

    void* start = (void*)mmap->p_start;
    void* end = (void*)((uintptr_t)start + ((uintptr_t)(mmap->num_pages) * EFI_PAGE_SIZE));

    trees::bookmark return_mark(start, end);

    // Check the mmap type
    if ((mmap->type > 0 && mmap->type < 5) || mmap->type == 7 || mmap->type == 14) {
        return_mark.flags |= trees::MARK_FREE;
    } else {
        return_mark.flags &= ~(trees::MARK_FREE);
    }

    return return_mark;
}

/**
 * @brief Initializes the kernel bookmarks from the given boot information
 * 
 * @param mb_info   Pointer to the boot info structure
 */
void memory_init(multiboot_boot_info* mb_info) {

    // Fill missing protected regions
    fill_protected_regions(mb_info);

    // Start up mark allocation on bootstrap bank
    mkalloc_start = (void*)bootstrap_bank;
    mkalloc_end = (void*)((uintptr_t)mkalloc_start + (256 * sizeof(trees::bookmark)));

    // Prepare the memory trees
    free_marks.size_sorted = true;
    used_marks.size_sorted = false;

    // Did GRUB give us a multiboot mmap?
    if ((uintptr_t)mb_info->mmap_tag) {

        // Loop through the mmap entries, adding them to the trees
        multiboot_mmap_entry* next_mmap = (&mb_info->mmap_tag->entries[0]);
        uintptr_t mmap_end = (uintptr_t)mb_info->mmap_tag + mb_info->mmap_tag->size;
        size_t entry_size = mb_info->mmap_tag->entry_size;
        while (1) {
            trees::bookmark* converted_mark = mkalloc();
            *converted_mark = mmap_to_mark(next_mmap);

            // Identity map the region
            paging::identity_map_region((uintptr_t)converted_mark->start, converted_mark->size);

            if (converted_mark->flags & trees::MARK_FREE) {
                // Check if this is a new greatest accessible address
                if (converted_mark->end > max_phys_address)
                    max_phys_address = converted_mark->end;

                free_marks.insert(converted_mark);

                clear_protected_regions(converted_mark->start, converted_mark->size);
            } else {
                used_marks.insert(converted_mark);
            }

            // Move to the next entry, if it exists
            next_mmap = (multiboot_mmap_entry*)((uintptr_t)next_mmap + entry_size);
            if ((uintptr_t)next_mmap >= mmap_end) {
                break;
            }
        }

    // Did GRUB give us an EFI mmap?
    } else if ((uintptr_t)mb_info->efi_mmap_tag) {

        // Loop through the mmap entries, adding them to the trees
        efi_mmap_entry* next_mmap = (efi_mmap_entry*)(&mb_info->efi_mmap_tag->efi_mmap[0]);
        uintptr_t mmap_end = (uintptr_t)mb_info->efi_mmap_tag + mb_info->efi_mmap_tag->size;
        size_t entry_size = mb_info->efi_mmap_tag->descr_size;
        while (1) {
            trees::bookmark* converted_mark = mkalloc();
            *converted_mark = mmap_to_mark(next_mmap);

            // Identity map the region
            paging::identity_map_region((uintptr_t)converted_mark->start, converted_mark->size);

            if (converted_mark->flags & trees::MARK_FREE) {
                // Check if this is a new greatest accessible address
                if (converted_mark->end > max_phys_address)
                    max_phys_address = converted_mark->end;

                free_marks.insert(converted_mark);
            } else {
                used_marks.insert(converted_mark);
            }

            // Move to the next entry, if it exists
            next_mmap = (efi_mmap_entry*)((uintptr_t)next_mmap + entry_size);
            if ((uintptr_t)next_mmap >= mmap_end) {
                break;
            }
        }

    // Do we at least have some info on the total memory?
    } else if ((uintptr_t)mb_info->meminfo_tag) {
        trees::bookmark* new_mark = mkalloc();
        new_mark->start = (void*)0x1;
        new_mark->end = (void*)((mb_info->meminfo_tag->mem_lower + mb_info->meminfo_tag->mem_upper) | 0UL);
        new_mark->size = (size_t)(mb_info->meminfo_tag->mem_lower + mb_info->meminfo_tag->mem_upper - 1);
        free_marks.insert(new_mark);

        // Identity map the region
        paging::identity_map_region(1, (size_t)(mb_info->meminfo_tag->mem_lower + mb_info->meminfo_tag->mem_upper - 1));

    // At least we know the kernel was loaded so assume all memory below the kernel end is available, but nothing else
    } else {
        trees::bookmark* new_mark = mkalloc();
        new_mark->start = (void*)0x1;
        new_mark->end = (void*)&kernel_end;
        new_mark->size = (size_t)((uintptr_t)&kernel_end - 1);
        free_marks.insert(new_mark);

        // Identity map the region
        paging::identity_map_region(1, (size_t)&kernel_end - 1);
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
    trees::bookmark* target_mark = free_marks.find_size(size);
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

void* balloc(size_t size, size_t alignment) {

    // Find suitable mark and remove it
    uintptr_t split_address = 0;
    trees::bookmark* target_mark = free_marks.find_aligned(size, alignment, split_address);
    free_marks.seperate(target_mark);

    void* return_address;
    trees::bookmark* used_mark;

    // Check if the mark needs to be split
    if (split_address) {
        // New mark for front section being cut off
        trees::bookmark* split_mark = mkalloc();
        split_mark->start = target_mark->start;
        split_mark->end = (void*)(split_address - 1);
        split_mark->size = (size_t)((split_address - 1) - (uintptr_t)(target_mark->start));
        split_mark->flags |= trees::MARK_FREE;
        free_marks.insert(split_mark);

        // Adjust old mark
        target_mark->start = (void*)split_address;
        target_mark->size -= split_mark->size;
    }

    // Standard procedure
    return_address = target_mark->start;

    // Determine if the mark should be put back or not
    if (target_mark->size > size) {

        // Needs to be put back, and a new mark is needed
        target_mark->start = (void*)((uintptr_t)target_mark->start + size);
        target_mark->size = (size_t)((uintptr_t)target_mark->end 
                                    - (uintptr_t)target_mark->start);
        target_mark->flags = 0;
        target_mark->balance = 0;
        free_marks.insert(target_mark);

        used_mark = mkalloc();
        used_mark->start = return_address;

    } else {

        // Don't need to put the mark back, so it can just be reused
        used_mark = target_mark;
    }

    // Prepare, and insert, the mark for the used space
    used_mark->size = size;
    used_mark->end = (void*)((uintptr_t)return_address + size);
    used_mark->flags &= ~(trees::MARK_FREE);
    used_marks.insert(used_mark);

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

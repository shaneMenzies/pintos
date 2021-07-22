/**
 * @file paging.cpp
 * @author Shane Menzies
 * @brief Virtual Memory Management
 * @date 06/05/21
 * 
 * 
 */

#include "paging.h"

namespace paging {

inline page_directory_pointer_table* get_page_directory_pointer_table(uintptr_t physical_address) {

    page_level_4_table* master_table = get_current_table();

    // Check if it has a matching PDP entry in the level 4 table
    int l4_index = (physical_address / page_directory_pointer_size);
    page_directory_pointer_table* target_pdp = (page_directory_pointer_table*)(master_table->data[l4_index] & real_address_bitmask);
    if (target_pdp == 0) {
        // Create new page directory pointer table
        target_pdp = (page_directory_pointer_table*)balloc(sizeof(page_directory_pointer_table), 4096);
        for (int i = 0; i < 512; i++) {
            target_pdp->data[i] = 0;
        }
        master_table->data[l4_index] = (uintptr_t)target_pdp | pdp_present | pdp_write_enabled;
    }

    return target_pdp;
};

inline page_directory_table* get_page_directory(uintptr_t physical_address) {

    page_directory_pointer_table* target_pdp = get_page_directory_pointer_table(physical_address);

    // Check if it has a matching PD entry in the PDP table
    physical_address %= page_directory_pointer_size;
    int pd_index = (physical_address / page_directory_size);
    page_directory_table* target_pd = (page_directory_table*)(target_pdp->data[pd_index] & real_address_bitmask);
    if (target_pd == 0) {
        // Create new page directory table
        target_pd = (page_directory_table*)balloc(sizeof(page_directory_table), 4096);
        for (int i = 0; i < 512; i++) {
            target_pd->data[i] = 0;
        }
        target_pdp->data[pd_index] = (uintptr_t)target_pd | pd_present | pd_write_enabled;
    }

    return target_pd;
};

inline page_table* get_page_table(uintptr_t physical_address) {

    page_directory_table* target_pd = get_page_directory(physical_address);

    // Return the corresponding page table
    physical_address %= page_directory_size;
    int pt_index = (physical_address / page_table_size);
    page_table* target_pt = (page_table*)(target_pd->data[pt_index] & real_address_bitmask);
    if (target_pt == 0) {
        // Create new paging table
        target_pt = (page_table*)balloc(sizeof(page_table), 4096);
        for (int i = 0; i < 512; i++) {
            target_pt->data[i] = 0;
        }
        target_pd->data[pt_index] = (uintptr_t)target_pt | pt_present | pt_write_enabled;
    }

    return target_pt;
};

inline page_entry* get_page(uintptr_t physical_address) {

    page_table* target_pt = get_page_table(physical_address);

    // Return the corresponding page entry
    physical_address %= page_table_size;
    return &(target_pt->data[physical_address / page_size]);
};

void identity_map_page(uintptr_t physical_address) {
    *(get_page(physical_address)) = ((physical_address & real_address_bitmask) | p_present | p_write_enabled);
    refresh_page((void*)physical_address);
}

void identity_map_pt(uintptr_t physical_address) {
    page_table* target_table = get_page_table(physical_address);

    // Loop through entries of the target table
    physical_address = (physical_address - (physical_address % page_table_size)) | p_present | p_write_enabled;
    for (int i = 0; i < 512; i++) {
        target_table->data[i] = physical_address;
        physical_address += page_size;
        refresh_page((void*)physical_address);
    }
}

void identity_map_pd(uintptr_t physical_address) {
    page_directory_table* target_directory = get_page_directory(physical_address);

    // Loop through the page tables
    physical_address = (physical_address - (physical_address % page_directory_size)) | p_present | p_write_enabled;
    for (int i = 0; i < 512; i++) {
        page_table* target_table = (page_table*)target_directory->data[i];

        // Check if this table exists
        if (target_table == 0) {
            // Doesn't exist, need to create it first
            target_table = (page_table*)balloc(sizeof(page_table), 4096);

            // Clear newly created table
            for (int i = 0; i < 512; i++) {
                target_table->data[i] = 0;
            }

            target_directory->data[i] = (uintptr_t)target_table | pt_present | pt_write_enabled;
        }

        // Identity map the entries of this table
        for (int i = 0; i < 512; i++) {
            target_table->data[i] = physical_address;
            physical_address += page_size;
        }
    }

    // Refresh the entire lookup directory
    refresh_all_pages();
}

void identity_map_region(uintptr_t target_address, size_t size) {

    // Change address (and size) to fall along page lines
    size_t change = target_address % page_size;
    target_address -= change;
    size += change;

    while (size) {
        // Break off into different levels of mapping
        if ((size > page_directory_size) && ((target_address & (page_directory_size - 1)) == 0)) {
            // Map next page directory
            identity_map_pd(target_address);
            target_address += page_directory_size;
            size -= page_directory_pointer_size;

        } else if ((size > page_table_size) && ((target_address & (page_table_size - 1)) == 0)) {
            // Map next page table
            identity_map_pt(target_address);
            target_address += page_table_size;
            size -= page_table_size;

        } else {
            // Map next page
            identity_map_page(target_address);
            target_address += page_size;
            if (size > page_size) {
                size -= page_size;
            } else {
                size = 0;
            }
        }
    }
}

}
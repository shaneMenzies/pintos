#ifndef ADDRESSING_H
#define ADDRESSING_H

#include "libk/vector.h"
#include "memory/paging.h"
#include "memory/sub_memory.h"

#include <stddef.h>
#include <stdint.h>

namespace paging {
void initialize_table_mapping(page_level_4_table* level_4_table);

struct address_space {
    std_k::vector<paging::page_level_4_table*> shared_tables;

    sub_mem_manager sub_page_memory;

    void* next_alloc_address = (void*)PAGE_SIZE;

    address_space() {}

    size_t get_new_table() {
        // Need to create new lvl4 table
        paging::page_level_4_table* new_table = new paging::page_level_4_table;

        // Initialize the bare minimum mapping in top 2 pdps
        paging::initialize_table_mapping(new_table);

        // Copy over higher half of kernel mapping
        for (int i = 0; i < 254; i++) {
            new_table->data[i + 256] = paging::pml4_table->data[i + 256];
        }

        // Add to vector of tables
        size_t used_index = shared_tables.size();
        shared_tables.push_back(new_table);

        return used_index;
    }

    void* get_new_address(size_t needed_size) {
        void* return_address = next_alloc_address;
        next_alloc_address
            = (void*)((uintptr_t)next_alloc_address + needed_size);

        // Round to next page-aligned address
        next_alloc_address = paging::round_up_page_aligned(next_alloc_address);
        return return_address;
    }

    void identity_map_page(uintptr_t target_address, size_t index = 0);
    void identity_map_pt(uintptr_t target_address, size_t index = 0);
    void identity_map_pd(uintptr_t target_address, size_t index = 0);

    void identity_map_region(uintptr_t target_address, size_t size,
                             size_t index = 0);

    void map_page_to(uintptr_t source_address, uintptr_t target_address,
                     size_t index = 0, bool lock_override = false);
    void map_pt_to(uintptr_t source_address, uintptr_t target_address,
                   size_t index = 0, bool lock_override = false);
    void map_pd_to(uintptr_t source_address, uintptr_t target_address,
                   size_t index = 0, bool lock_override = false);

    void map_region_to(uintptr_t source_address, uintptr_t target_address,
                       size_t size, size_t index = 0,
                       bool lock_override = false);

    inline page_directory_pointer_table*
        get_page_directory_pointer_table(uintptr_t virtual_address,
                                         size_t    index         = 0,
                                         bool      lock_override = false) {

        // Check if it has a matching PDP entry in the level 4 table
        int  l4_index = ((virtual_address % page_level_4_table_size)
                        / page_directory_pointer_size);
        bool exists   = (shared_tables[index]->data[l4_index] != 0);
        if (!exists) {
            // Create new page directory pointer table
            uintptr_t p_address = (uintptr_t)palloc(lock_override);
            page_directory_pointer_table* target_pdp = &pdp_tables[l4_index];
            map_page_to(p_address, (uintptr_t)target_pdp, index, lock_override);
            for (int i = 0; i < 512; i++) { target_pdp->data[i] = 0; }

            // Set into table(s)
            // Indices 0-255 cover the user-space
            if (l4_index < 255) {
                // Copy to all
                for (size_t i = 0; i < shared_tables.size(); i++) {
                    shared_tables[i]->data[l4_index] = (uintptr_t)p_address
                                                       | pdp_present
                                                       | pdp_write_enabled;
                }
            } else {
                pml4_table->data[l4_index]
                    = (uintptr_t)p_address | pdp_present | pdp_write_enabled;
            }
        }

        return &pdp_tables[l4_index];
    };

    inline page_directory_table*
        get_page_directory(uintptr_t virtual_address, size_t index = 0,
                           bool lock_override = false) {

        page_directory_pointer_table* parent_table
            = get_page_directory_pointer_table(virtual_address, index,
                                               lock_override);

        // Check if it has a matching PD entry in the PDP table
        int master_index = ((virtual_address % page_level_4_table_size)
                            / page_directory_size);
        virtual_address %= page_directory_pointer_size;
        int  parent_index = (virtual_address / page_directory_size);
        bool exists       = (parent_table->data[parent_index] != 0);

        if (!exists) {
            // Create new page directory table
            uintptr_t             p_address = (uintptr_t)palloc(lock_override);
            page_directory_table* target_pd = &pd_tables[master_index];
            map_page_to(p_address, (uintptr_t)target_pd, index, lock_override);
            for (int i = 0; i < 512; i++) { target_pd->data[i] = 0; }
            parent_table->data[parent_index]
                = (uintptr_t)p_address | pd_present | pd_write_enabled;
        }

        return &pd_tables[master_index];
    };

    inline page_table* get_page_table(uintptr_t virtual_address,
                                      size_t    index         = 0,
                                      bool      lock_override = false) {

        page_directory_table* parent_table
            = get_page_directory(virtual_address, index, lock_override);

        // Check if it has a matching PT entry in the PD table
        int master_index
            = ((virtual_address % page_level_4_table_size) / page_table_size);
        virtual_address %= page_directory_size;
        int  parent_index = (virtual_address / page_table_size);
        bool exists       = (parent_table->data[parent_index] != 0);

        if (!exists) {
            // Create new page directory pointer table
            uintptr_t   p_address = (uintptr_t)palloc(lock_override);
            page_table* target_pt = &pt_tables[master_index];
            map_page_to(p_address, (uintptr_t)target_pt, lock_override);
            for (int i = 0; i < 512; i++) { target_pt->data[i] = 0; }
            parent_table->data[parent_index]
                = (uintptr_t)p_address | pt_present | pt_write_enabled;
        }

        return &pt_tables[master_index];
    };

    inline page_entry* get_page(uintptr_t virtual_address, size_t index = 0,
                                bool lock_override = false) {

        page_table* target_pt
            = get_page_table(virtual_address, index, lock_override);

        // Return the corresponding page entry
        virtual_address %= page_table_size;
        return &(target_pt->data[virtual_address / PAGE_SIZE]);
    };

    inline uintptr_t virt_to_phys(void* virtual_address, size_t index = 0) {
        return (*get_page((uintptr_t)(virtual_address), index)
                & real_address_bitmask);
    };
};

extern "C" {
extern void* kernel_end;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winit-self"

// Alternate version of address space for kernel's portion of memory,
//  which should be able to be accessed using any pml4 table
struct kernel_only_address_space {

    sub_mem_manager sub_page_memory;

    void* next_alloc_address;

    kernel_only_address_space()
        : sub_page_memory(sub_page_memory)
        , next_alloc_address(next_alloc_address) {}

    void initialize();

    void* get_new_address(size_t needed_size) {
        void* return_address = next_alloc_address;
        next_alloc_address
            = (void*)((uintptr_t)next_alloc_address + needed_size);

        // Round to next page-aligned address
        next_alloc_address = paging::round_up_page_aligned(next_alloc_address);
        return return_address;
    }

    void identity_map_page(uintptr_t target_address);
    void identity_map_pt(uintptr_t target_address);
    void identity_map_pd(uintptr_t target_address);

    void identity_map_region(uintptr_t target_address, size_t size);

    void map_page_to(uintptr_t source_address, uintptr_t target_address,
                     bool lock_override = false);
    void map_pt_to(uintptr_t source_address, uintptr_t target_address,
                   bool lock_override = false);
    void map_pd_to(uintptr_t source_address, uintptr_t target_address,
                   bool lock_override = false);

    void map_region_to(uintptr_t source_address, uintptr_t target_address,
                       size_t size, bool lock_override = false);

    inline page_directory_pointer_table*
        get_page_directory_pointer_table(uintptr_t virtual_address,
                                         bool      lock_override = false) {

        // Check if it has a matching PDP entry in the level 4 table
        int  l4_index = ((virtual_address % page_level_4_table_size)
                        / page_directory_pointer_size);
        bool exists   = (pml4_table->data[l4_index] != 0);
        if (!exists) {
            // Create new page directory pointer table
            uintptr_t p_address = (uintptr_t)palloc(lock_override);
            page_directory_pointer_table* target_pdp = &pdp_tables[l4_index];
            map_page_to(p_address, (uintptr_t)target_pdp, lock_override);
            for (int i = 0; i < 512; i++) { target_pdp->data[i] = 0; }

            // Set into table(s)
            pml4_table->data[l4_index]
                = (uintptr_t)p_address | pdp_present | pdp_write_enabled;
        }

        return &pdp_tables[l4_index];
    };

    inline page_directory_table* get_page_directory(uintptr_t virtual_address,
                                                    bool      lock_override
                                                    = false) {

        page_directory_pointer_table* parent_table
            = get_page_directory_pointer_table(virtual_address, lock_override);

        // Check if it has a matching PD entry in the PDP table
        int master_index = ((virtual_address % page_level_4_table_size)
                            / page_directory_size);
        virtual_address %= page_directory_pointer_size;
        int  parent_index = (virtual_address / page_directory_size);
        bool exists       = (parent_table->data[parent_index] != 0);

        if (!exists) {
            // Create new page directory table
            uintptr_t             p_address = (uintptr_t)palloc(lock_override);
            page_directory_table* target_pd = &pd_tables[master_index];
            map_page_to(p_address, (uintptr_t)target_pd, lock_override);
            for (int i = 0; i < 512; i++) { target_pd->data[i] = 0; }
            parent_table->data[parent_index]
                = (uintptr_t)p_address | pd_present | pd_write_enabled;
        }

        return &pd_tables[master_index];
    };

    inline page_table* get_page_table(uintptr_t virtual_address,
                                      bool      lock_override = false) {

        page_directory_table* parent_table
            = get_page_directory(virtual_address, lock_override);

        // Check if it has a matching PT entry in the PD table
        int master_index
            = ((virtual_address % page_level_4_table_size) / page_table_size);
        virtual_address %= page_directory_size;
        int  parent_index = (virtual_address / page_table_size);
        bool exists       = (parent_table->data[parent_index] != 0);

        if (!exists) {
            // Create new page directory pointer table
            uintptr_t   p_address = (uintptr_t)palloc(lock_override);
            page_table* target_pt = &pt_tables[master_index];
            map_page_to(p_address, (uintptr_t)target_pt, lock_override);
            for (int i = 0; i < 512; i++) { target_pt->data[i] = 0; }
            parent_table->data[parent_index]
                = (uintptr_t)p_address | pt_present | pt_write_enabled;
        }

        return &pt_tables[master_index];
    };

    inline page_entry* get_page(uintptr_t virtual_address,
                                bool      lock_override = false) {

        page_table* target_pt = get_page_table(virtual_address, lock_override);

        // Return the corresponding page entry
        virtual_address %= page_table_size;
        return &(target_pt->data[virtual_address / PAGE_SIZE]);
    };

    inline uintptr_t virt_to_phys(void* virtual_address) {
        return (*get_page((uintptr_t)(virtual_address)) & real_address_bitmask);
    };
};

#pragma GCC diagnostic pop

extern kernel_only_address_space kernel_address_space;
} // namespace paging

#endif

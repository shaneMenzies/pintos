/**
 * @file addressing.cpp
 * @author Shane Menzies
 * @brief Virtual address management
 * @date 10/25/21
 *
 *
 */

#include "addressing.h"

#include "sub_memory.h"

namespace paging {

kernel_only_address_space kernel_address_space;

/*
 * Initial mapping layout:
 *
 *  (table) (index) (location) (mapped address) (needed pt entry)
 *
 *  PML4T                       0xff7f bfdf f000    pt[0]
 *
 *  PDP 0x1FE (pdp[0])          0xff7f bfff e000    pt[1]
 *      PD 0x1FE (pd[0])        0xff7f ffdf e000    pt[2]
 *          PT 0x1FE (pt[0])    0xffff bfdf e000    pt[4]
 *          PT 0x1FF (pt[1])    0xffff bfdf f000    pt[4]
 *      PD 0x1FF (pd[1])        0xff7f ffdf f000    pt[2]
 *          PT 0x1FE (pt[2])    0xffff bfff e000    pt[5]
 *          PT 0x1FF (pt[3])    0xffff bfff f000    pt[5]
 *
 *  PDP 0x1FF (pdp[1])          0xff7f bfff f000    pt[1]
 *      PD 0x1FE (pd[2])        0xff7f ffff e000    pt[3]
 *          PT 0x1FE (pt[4])    0xffff ffdf e000    pt[6]
 *          PT 0x1FF (pt[5])    0xffff ffdf f000    pt[6]
 *      PD 0x1FF (pd[3])        0xff7f ffff f000    pt[3]
 *          PT 0x1FE (pt[6])    0xffff ffff e000    pt[7]
 *          PT 0x1FF (pt[7])    0xffff ffff f000    pt[7]
 *
 * (pt index) (lower_lim)-(upper_lim)
 *
 *  0   0xff7f bfc0 0000 - 0xff7f bfe0 0000
 *  1   0xff7f bfe0 0000 - 0xff7f c000 0000
 *  2   0xff7f ffc0 0000 - 0xff7f ffe0 0000
 *  3   0xff7f ffe0 0000 - 0xff80 0000 0000
 *  4   0xffff bfc0 0000 - 0xffff bfe0 0000
 *  5   0xffff bfe0 0000 - 0xffff c000 0000
 *  6   0xffff ffc0 0000 - 0xffff ffe0 0000
 *  7   0xffff ffe0 0000 - 0x0000 0000 0000
 *
 */
void initialize_table_mapping(page_level_4_table* level_4_table) {

    page_directory_pointer_table* pdp = new page_directory_pointer_table[2];
    page_directory_table*         pd  = new page_directory_table[4];
    page_table*                   pt  = new page_table[8];

    // Clear all tables
    for (int i = 0; i < 512; i++) level_4_table->data[i] = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 512; j++) { pdp[i].data[j] = 0; }
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 512; j++) { pd[i].data[j] = 0; }
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 512; j++) { pt[i].data[j] = 0; }
    }

    // Prepare the initial tables
    // PML4T
    level_4_table->data[0x1fe]
        = ((kernel_address_space.virt_to_phys(&pdp[0]) & real_address_bitmask)
           | pd_present | pd_write_enabled);
    level_4_table->data[0x1ff]
        = ((kernel_address_space.virt_to_phys(&pdp[1]) & real_address_bitmask)
           | pd_present | pd_write_enabled);

    // PDPs
    pdp[0].data[0x1fe]
        = ((kernel_address_space.virt_to_phys(&pd[0]) & real_address_bitmask)
           | pd_present | pd_write_enabled);
    pdp[0].data[0x1ff]
        = ((kernel_address_space.virt_to_phys(&pd[1]) & real_address_bitmask)
           | pd_present | pd_write_enabled);

    pdp[1].data[0x1fe]
        = ((kernel_address_space.virt_to_phys(&pd[2]) & real_address_bitmask)
           | pd_present | pd_write_enabled);
    pdp[1].data[0x1ff]
        = ((kernel_address_space.virt_to_phys(&pd[3]) & real_address_bitmask)
           | pd_present | pd_write_enabled);

    // PDs
    pd[0].data[0x1fe]
        = ((kernel_address_space.virt_to_phys(&pt[0]) & real_address_bitmask)
           | pt_present | pt_write_enabled);
    pd[0].data[0x1ff]
        = ((kernel_address_space.virt_to_phys(&pt[1]) & real_address_bitmask)
           | pt_present | pt_write_enabled);

    pd[1].data[0x1fe]
        = ((kernel_address_space.virt_to_phys(&pt[2]) & real_address_bitmask)
           | pt_present | pt_write_enabled);
    pd[1].data[0x1ff]
        = ((kernel_address_space.virt_to_phys(&pt[3]) & real_address_bitmask)
           | pt_present | pt_write_enabled);

    pd[2].data[0x1fe]
        = ((kernel_address_space.virt_to_phys(&pt[4]) & real_address_bitmask)
           | pt_present | pt_write_enabled);
    pd[2].data[0x1ff]
        = ((kernel_address_space.virt_to_phys(&pt[5]) & real_address_bitmask)
           | pt_present | pt_write_enabled);

    pd[3].data[0x1fe]
        = ((kernel_address_space.virt_to_phys(&pt[6]) & real_address_bitmask)
           | pt_present | pt_write_enabled);
    pd[3].data[0x1ff]
        = ((kernel_address_space.virt_to_phys(&pt[7]) & real_address_bitmask)
           | pt_present | pt_write_enabled);

    // Mappings in PTs
    // PML4T
    pt[0].data[((uintptr_t)pml4_table % page_table_size) / PAGE_SIZE]
        = ((kernel_address_space.virt_to_phys(level_4_table)
            & real_address_bitmask)
           | p_present | p_write_enabled);

    // PDPs
    pt[1].data[((uintptr_t)&pdp_tables[0x1fe] % page_table_size) / PAGE_SIZE]
        = ((kernel_address_space.virt_to_phys(&pdp[0]) & real_address_bitmask)
           | p_present | p_write_enabled);
    pt[1].data[((uintptr_t)&pdp_tables[0x1ff] % page_table_size) / PAGE_SIZE]
        = ((kernel_address_space.virt_to_phys(&pdp[1]) & real_address_bitmask)
           | p_present | p_write_enabled);

    // PDs
    pt[2]
        .data[((uintptr_t)&pd_tables[(0x200 * 0x1fe) + 0x1fe] % page_table_size)
              / PAGE_SIZE]
        = ((kernel_address_space.virt_to_phys(&pd[0]) & real_address_bitmask)
           | p_present | p_write_enabled);
    pt[2]
        .data[((uintptr_t)&pd_tables[(0x200 * 0x1fe) + 0x1ff] % page_table_size)
              / PAGE_SIZE]
        = ((kernel_address_space.virt_to_phys(&pd[1]) & real_address_bitmask)
           | p_present | p_write_enabled);

    pt[3]
        .data[((uintptr_t)&pd_tables[(0x200 * 0x1ff) + 0x1fe] % page_table_size)
              / PAGE_SIZE]
        = ((kernel_address_space.virt_to_phys(&pd[2]) & real_address_bitmask)
           | p_present | p_write_enabled);
    pt[3]
        .data[((uintptr_t)&pd_tables[(0x200 * 0x1ff) + 0x1ff] % page_table_size)
              / PAGE_SIZE]
        = ((kernel_address_space.virt_to_phys(&pd[3]) & real_address_bitmask)
           | p_present | p_write_enabled);

    // PTs
    pt[4].data[((uintptr_t)&pt_tables[(0x40000 * 0x1fe) + (0x200 * 0x1fe)
                                      + 0x1fe]
                % page_table_size)
               / PAGE_SIZE]
        = ((kernel_address_space.virt_to_phys(&pt[0]) & real_address_bitmask)
           | p_present | p_write_enabled);
    pt[4].data[((uintptr_t)&pt_tables[(0x40000 * 0x1fe) + (0x200 * 0x1fe)
                                      + 0x1ff]
                % page_table_size)
               / PAGE_SIZE]
        = ((kernel_address_space.virt_to_phys(&pt[1]) & real_address_bitmask)
           | p_present | p_write_enabled);

    pt[5].data[((uintptr_t)&pt_tables[(0x40000 * 0x1fe) + (0x200 * 0x1ff)
                                      + 0x1fe]
                % page_table_size)
               / PAGE_SIZE]
        = ((kernel_address_space.virt_to_phys(&pt[2]) & real_address_bitmask)
           | p_present | p_write_enabled);
    pt[5].data[((uintptr_t)&pt_tables[(0x40000 * 0x1fe) + (0x200 * 0x1ff)
                                      + 0x1ff]
                % page_table_size)
               / PAGE_SIZE]
        = ((kernel_address_space.virt_to_phys(&pt[3]) & real_address_bitmask)
           | p_present | p_write_enabled);

    pt[6].data[((uintptr_t)&pt_tables[(0x40000 * 0x1ff) + (0x200 * 0x1fe)
                                      + 0x1fe]
                % page_table_size)
               / PAGE_SIZE]
        = ((kernel_address_space.virt_to_phys(&pt[4]) & real_address_bitmask)
           | p_present | p_write_enabled);
    pt[6].data[((uintptr_t)&pt_tables[(0x40000 * 0x1ff) + (0x200 * 0x1fe)
                                      + 0x1ff]
                % page_table_size)
               / PAGE_SIZE]
        = ((kernel_address_space.virt_to_phys(&pt[5]) & real_address_bitmask)
           | p_present | p_write_enabled);

    pt[7].data[((uintptr_t)&pt_tables[(0x40000 * 0x1ff) + (0x200 * 0x1ff)
                                      + 0x1fe]
                % page_table_size)
               / PAGE_SIZE]
        = ((kernel_address_space.virt_to_phys(&pt[6]) & real_address_bitmask)
           | p_present | p_write_enabled);
    pt[7].data[((uintptr_t)&pt_tables[(0x40000 * 0x1ff) + (0x200 * 0x1ff)
                                      + 0x1ff]
                % page_table_size)
               / PAGE_SIZE]
        = ((kernel_address_space.virt_to_phys(&pt[7]) & real_address_bitmask)
           | p_present | p_write_enabled);

    // TODO: Clear mapping for newly created tables, while leaving them
    // allocated
}

void address_space::identity_map_page(uintptr_t target_address) {
    *(get_page(target_address)) = ((target_address & real_address_bitmask)
                                   | p_present | p_write_enabled);
    refresh_page((void*)target_address);
}

void address_space::identity_map_pt(uintptr_t target_address) {
    page_table* target_table = get_page_table(target_address);

    // Loop through entries of the target table
    target_address = (target_address - (target_address % page_table_size))
                     | p_present | p_write_enabled;
    for (int i = 0; i < 512; i++) {
        target_table->data[i] = target_address;
        target_address += PAGE_SIZE;
        refresh_page((void*)target_address);
    }
}

void address_space::identity_map_pd(uintptr_t target_address) {
    uintptr_t             original_address = target_address;
    page_directory_table* target_directory = get_page_directory(target_address);

    // Loop through the page tables
    target_address = (target_address - (target_address % page_directory_size))
                     | p_present | p_write_enabled;
    for (int i = 0; i < 512; i++) {
        page_table* target_table = (page_table*)target_directory->data[i];

        // Check if this table exists
        if (target_table == 0) {
            // Doesn't exist, need to create it first
            uintptr_t p_address = palloc();
            target_table = &pt_tables[(original_address / page_table_size)];
            map_page_to(p_address, (uintptr_t)target_table);
            for (int j = 0; j < 512; j++) { target_table->data[j] = 0; }

            target_directory->data[i]
                = p_address | pt_present | pt_write_enabled;
        }

        // Identity map the entries of this table
        for (int j = 0; j < 512; j++) {
            target_table->data[j] = target_address;
            target_address += PAGE_SIZE;
            refresh_page((void*)target_address);
        }
    }
}

void address_space::identity_map_region(uintptr_t target_address, size_t size) {

    // Ensure that addresses (and size) fall along page lines
    size += (target_address % PAGE_SIZE);
    target_address -= (target_address % PAGE_SIZE);

    while (size) {
        // Break off into different levels of mapping
        if ((size > page_directory_size)
            && ((target_address & (page_directory_size - 1)) == 0)) {
            // Map next page directory
            identity_map_pd(target_address);
            target_address += page_directory_size;
            if (size > page_directory_size) {
                size -= page_directory_size;
            } else {
                size = 0;
            }

        } else if ((size > page_table_size)
                   && ((target_address & (page_table_size - 1)) == 0)) {
            // Map next page table
            identity_map_pt(target_address);
            target_address += page_table_size;
            if (size > page_table_size) {
                size -= page_table_size;
            } else {
                size = 0;
            }

        } else {
            // Map next page
            identity_map_page(target_address);
            target_address += PAGE_SIZE;
            if (size > PAGE_SIZE) {
                size -= PAGE_SIZE;
            } else {
                size = 0;
            }
        }
    }
}

void address_space::map_page_to(uintptr_t source_address,
                                uintptr_t target_address, bool lock_override) {
    *(get_page(target_address, lock_override))
        = ((source_address & real_address_bitmask) | p_present
           | p_write_enabled);
    refresh_page((void*)target_address);
}

void address_space::map_pt_to(uintptr_t source_address,
                              uintptr_t target_address, bool lock_override) {
    page_table* target_table = get_page_table(target_address, lock_override);

    // Loop through entries of the target table
    source_address = (source_address - (source_address % page_table_size))
                     | p_present | p_write_enabled;
    for (int i = 0; i < 512; i++) {
        target_table->data[i] = source_address;
        source_address += PAGE_SIZE;
        refresh_page((void*)target_address);
    }
}

void address_space::map_pd_to(uintptr_t source_address,
                              uintptr_t target_address, bool lock_override) {
    page_directory_table* target_directory
        = get_page_directory(target_address, lock_override);

    // Loop through the page tables
    source_address = (source_address - (source_address % page_directory_size))
                     | p_present | p_write_enabled;
    for (int i = 0; i < 512; i++) {
        page_table* target_table = (page_table*)target_directory->data[i];

        // Check if this table exists
        if (target_table == 0) {
            // Doesn't exist, need to create it first
            uintptr_t p_address = (uintptr_t)palloc(lock_override);
            target_table = &pt_tables[(target_address / page_table_size)];
            map_page_to(p_address, (uintptr_t)target_table, lock_override);
            for (int j = 0; j < 512; j++) { target_table->data[j] = 0; }

            target_directory->data[i]
                = p_address | pt_present | pt_write_enabled;
        }

        // Map the entries of this table
        for (int j = 0; j < 512; j++) {
            target_table->data[j] = source_address;
            source_address += PAGE_SIZE;
            refresh_page((void*)target_address);
        }
    }
}

void address_space::map_region_to(uintptr_t source_address,
                                  uintptr_t target_address, size_t size,
                                  bool lock_override) {

    // Ensure that addresses (and size) fall along page lines
    size += (target_address % PAGE_SIZE);
    target_address -= (target_address % PAGE_SIZE);
    source_address -= (source_address % PAGE_SIZE);

    while (size) {
        // Break off into different levels of mapping
        if ((size > page_directory_size)
            && ((target_address & (page_directory_size - 1)) == 0)) {
            // Map next page directory
            map_pd_to(source_address, target_address, lock_override);
            source_address += page_directory_size;
            target_address += page_directory_size;
            if (size > page_directory_size) {
                size -= page_directory_size;
            } else {
                size = 0;
            }

        } else if ((size > page_table_size)
                   && ((target_address & (page_table_size - 1)) == 0)) {
            // Map next page table
            map_pt_to(source_address, target_address, lock_override);
            source_address += page_table_size;
            target_address += page_table_size;
            if (size > page_table_size) {
                size -= page_table_size;
            } else {
                size = 0;
            }

        } else {
            // Map next page
            map_page_to(source_address, target_address, lock_override);
            source_address += PAGE_SIZE;
            target_address += PAGE_SIZE;
            if (size > PAGE_SIZE) {
                size -= PAGE_SIZE;
            } else {
                size = 0;
            }
        }
    }
}

void kernel_only_address_space::initialize() {

    new (&sub_page_memory) sub_mem_manager();

    next_alloc_address
        = (void*)(((uintptr_t)&kernel_end & ~(PAGE_SIZE - 1)) + PAGE_SIZE);

    // Need to make sure that all kernel space PDPs exist
    //      PDP indices 256-509
    for (size_t i = 256; i < 510; i++) {
        get_page_directory_pointer_table(
            (uintptr_t)(page_directory_pointer_size * i));
    }
}

void kernel_only_address_space::identity_map_page(uintptr_t target_address) {
    *(get_page(target_address)) = ((target_address & real_address_bitmask)
                                   | p_present | p_write_enabled);
    refresh_page((void*)target_address);
}

void kernel_only_address_space::identity_map_pt(uintptr_t target_address) {
    page_table* target_table = get_page_table(target_address);

    // Loop through entries of the target table
    target_address = (target_address - (target_address % page_table_size))
                     | p_present | p_write_enabled;
    for (int i = 0; i < 512; i++) {
        target_table->data[i] = target_address;
        target_address += PAGE_SIZE;
        refresh_page((void*)target_address);
    }
}

void kernel_only_address_space::identity_map_pd(uintptr_t target_address) {
    uintptr_t             original_address = target_address;
    page_directory_table* target_directory = get_page_directory(target_address);

    // Loop through the page tables
    target_address = (target_address - (target_address % page_directory_size))
                     | p_present | p_write_enabled;
    for (int i = 0; i < 512; i++) {
        page_table* target_table = (page_table*)target_directory->data[i];

        // Check if this table exists
        if (target_table == 0) {
            // Doesn't exist, need to create it first
            uintptr_t p_address = palloc();
            target_table = &pt_tables[(original_address / page_table_size)];
            map_page_to(p_address, (uintptr_t)target_table);
            for (int j = 0; j < 512; j++) { target_table->data[j] = 0; }

            target_directory->data[i]
                = p_address | pt_present | pt_write_enabled;
        }

        // Identity map the entries of this table
        for (int j = 0; j < 512; j++) {
            target_table->data[j] = target_address;
            target_address += PAGE_SIZE;
            refresh_page((void*)target_address);
        }
    }
}

void kernel_only_address_space::identity_map_region(uintptr_t target_address,
                                                    size_t    size) {

    // Ensure that addresses (and size) fall along page lines
    size += (target_address % PAGE_SIZE);
    target_address -= (target_address % PAGE_SIZE);

    while (size) {
        // Break off into different levels of mapping
        if ((size > page_directory_size)
            && ((target_address & (page_directory_size - 1)) == 0)) {
            // Map next page directory
            identity_map_pd(target_address);
            target_address += page_directory_size;
            if (size > page_directory_size) {
                size -= page_directory_size;
            } else {
                size = 0;
            }

        } else if ((size > page_table_size)
                   && ((target_address & (page_table_size - 1)) == 0)) {
            // Map next page table
            identity_map_pt(target_address);
            target_address += page_table_size;
            if (size > page_table_size) {
                size -= page_table_size;
            } else {
                size = 0;
            }

        } else {
            // Map next page
            identity_map_page(target_address);
            target_address += PAGE_SIZE;
            if (size > PAGE_SIZE) {
                size -= PAGE_SIZE;
            } else {
                size = 0;
            }
        }
    }
}

void kernel_only_address_space::map_page_to(uintptr_t source_address,
                                            uintptr_t target_address,
                                            bool      lock_override) {
    *(get_page(target_address, lock_override))
        = ((source_address & real_address_bitmask) | p_present
           | p_write_enabled);
    refresh_page((void*)target_address);
}

void kernel_only_address_space::map_pt_to(uintptr_t source_address,
                                          uintptr_t target_address,
                                          bool      lock_override) {
    page_table* target_table = get_page_table(target_address, lock_override);

    // Loop through entries of the target table
    source_address = (source_address - (source_address % page_table_size))
                     | p_present | p_write_enabled;
    for (int i = 0; i < 512; i++) {
        target_table->data[i] = source_address;
        source_address += PAGE_SIZE;
        refresh_page((void*)target_address);
    }
}

void kernel_only_address_space::map_pd_to(uintptr_t source_address,
                                          uintptr_t target_address,
                                          bool      lock_override) {
    page_directory_table* target_directory
        = get_page_directory(target_address, lock_override);

    // Loop through the page tables
    source_address = (source_address - (source_address % page_directory_size))
                     | p_present | p_write_enabled;
    for (int i = 0; i < 512; i++) {
        page_table* target_table = (page_table*)target_directory->data[i];

        // Check if this table exists
        if (target_table == 0) {
            // Doesn't exist, need to create it first
            uintptr_t p_address = (uintptr_t)palloc(lock_override);
            target_table = &pt_tables[(target_address / page_table_size)];
            map_page_to(p_address, (uintptr_t)target_table, lock_override);
            for (int j = 0; j < 512; j++) { target_table->data[j] = 0; }

            target_directory->data[i]
                = p_address | pt_present | pt_write_enabled;
        }

        // Map the entries of this table
        for (int j = 0; j < 512; j++) {
            target_table->data[j] = source_address;
            source_address += PAGE_SIZE;
            refresh_page((void*)target_address);
        }
    }
}

void kernel_only_address_space::map_region_to(uintptr_t source_address,
                                              uintptr_t target_address,
                                              size_t size, bool lock_override) {

    // Ensure that addresses (and size) fall along page lines
    size += (target_address % PAGE_SIZE);
    target_address -= (target_address % PAGE_SIZE);
    source_address -= (source_address % PAGE_SIZE);

    while (size) {
        // Break off into different levels of mapping
        if ((size > page_directory_size)
            && ((target_address & (page_directory_size - 1)) == 0)) {
            // Map next page directory
            map_pd_to(source_address, target_address, lock_override);
            source_address += page_directory_size;
            target_address += page_directory_size;
            if (size > page_directory_size) {
                size -= page_directory_size;
            } else {
                size = 0;
            }

        } else if ((size > page_table_size)
                   && ((target_address & (page_table_size - 1)) == 0)) {
            // Map next page table
            map_pt_to(source_address, target_address, lock_override);
            source_address += page_table_size;
            target_address += page_table_size;
            if (size > page_table_size) {
                size -= page_table_size;
            } else {
                size = 0;
            }

        } else {
            // Map next page
            map_page_to(source_address, target_address, lock_override);
            source_address += PAGE_SIZE;
            target_address += PAGE_SIZE;
            if (size > PAGE_SIZE) {
                size -= PAGE_SIZE;
            } else {
                size = 0;
            }
        }
    }
}
} // namespace paging

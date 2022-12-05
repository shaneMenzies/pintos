/**
 * @file bootstrap_64.cpp
 * @author Shane Menzies
 * @brief 64-bit boot loader functions
 * @date 10/31/21
 *
 *
 */

#include "system/elf_64.h"
#include "system/loader.h"
#include "system/multiboot.h"
#include "system/pintos_std.h"

namespace paging {

#define PAGE_SIZE 0x1000
constexpr uint64_t page_table_size             = (PAGE_SIZE * 512);
constexpr uint64_t page_directory_size         = (page_table_size * 512);
constexpr uint64_t page_directory_pointer_size = (page_directory_size * 512);
constexpr uint64_t page_level_4_table_size
    = (page_directory_pointer_size * 512);
constexpr uintptr_t real_address_bitmask = ~(0xfff);

typedef uint64_t page_entry;
typedef struct {
    page_entry data[512];
} page_table;

typedef uint64_t page_table_entry;
typedef struct {
    page_table_entry data[512];
} page_directory_table;

typedef uint64_t page_directory_entry;
typedef struct {
    page_directory_entry data[512];
} page_directory_pointer_table;

typedef uint64_t page_directory_pointer_table_entry;
typedef struct {
    page_directory_pointer_table_entry data[512];
} page_level_4_table;

constexpr uintptr_t       table_start_address = 0xffffff7fbfdff000;
page_level_4_table* const pml4_table
    = (page_level_4_table*)table_start_address; // 1 pml4 table
page_directory_pointer_table* const pdp_tables
    = (page_directory_pointer_table*)(table_start_address
                                      + PAGE_SIZE); // 512 pdp tables
page_directory_table* const pd_tables
    = (page_directory_table*)(table_start_address
                              + (PAGE_SIZE * 513)); // 262144 pd tables
page_table* const pt_tables
    = (page_table*)(table_start_address
                    + (PAGE_SIZE
                       * ((512 * 512) + 513))); // 134217728 page tables

enum page_flags {
    p_present               = (1 << 0),
    p_write_enabled         = (1 << 1),
    p_supervisor_restricted = (1 << 2),
    p_write_through         = (1 << 3),
    p_cache_disabled        = (1 << 4),
    p_accessed              = (1 << 5),

    p_dirty  = (1 << 6),
    p_global = (1 << 8)
};

enum page_table_flags {
    pt_present               = (1 << 0),
    pt_write_enabled         = (1 << 1),
    pt_supervisor_restricted = (1 << 2),
    pt_write_through         = (1 << 3),
    pt_cache_disabled        = (1 << 4),
    pt_accessed              = (1 << 5),

    pt_large_pages = (1 << 7),
};

enum page_directory_flags {
    pd_present       = (1 << 0),
    pd_write_enabled = (1 << 1)
};

enum page_directory_pointer_flags {
    pdp_present       = (1 << 0),
    pdp_write_enabled = (1 << 1)
};

inline page_level_4_table* get_current_table() {

    uint64_t cr3_value;

    asm volatile("movq %%cr3, %%rax\n\t\
         movq %%rax, %[value]"
                 : [value] "=r"(cr3_value)
                 :
                 : "rax");

    return (page_level_4_table*)(uintptr_t)(cr3_value | 0UL);
};

inline void refresh_all_pages() {
    asm volatile("movq %%cr3, %%rax\n\t\
         movq %%rax, %%cr3"
                 :
                 :
                 : "rax");
};

inline void refresh_page(void* target) {
    asm volatile("invlpg %[target]" : : [target] "m"(target));
};

#ifndef KERNEL_SIZE
    #define KERNEL_SIZE 0
#endif

// Bootstrap table counts based on size of the kernel + amount needed to set up
// basic mapping
constexpr unsigned int KERNEL_PT_NUM  = KERNEL_SIZE / 0x200000 + 11;
constexpr unsigned int KERNEL_PD_NUM  = KERNEL_SIZE / 0x40000000 + 6;
constexpr unsigned int KERNEL_PDP_NUM = KERNEL_SIZE / 0x8000000000 + 3;

page_directory_pointer_table bootstrap_pdp[KERNEL_PDP_NUM]
    __attribute__((aligned(PAGE_SIZE)));
page_directory_table bootstrap_pd[KERNEL_PD_NUM]
    __attribute__((aligned(PAGE_SIZE)));
page_table   bootstrap_pt[KERNEL_PT_NUM] __attribute__((aligned(PAGE_SIZE)));
unsigned int bootstrap_pd_index  = 0;
unsigned int bootstrap_pdp_index = 0;
unsigned int bootstrap_pt_index  = 0;

void map_page_to(uintptr_t source_address, uintptr_t target_address);
void map_pt_to(uintptr_t source_address, uintptr_t target_address);
void map_pd_to(uintptr_t source_address, uintptr_t target_address);

void map_region_to(uintptr_t source_address, uintptr_t target_address,
                   size_t size);

inline page_directory_pointer_table*
    get_page_directory_pointer_table(uintptr_t virtual_address) {

    // Check if it has a matching PDP entry in the level 4 table
    int  l4_index = ((virtual_address % page_level_4_table_size)
                    / page_directory_pointer_size);
    bool exists   = (pml4_table->data[l4_index] != 0);
    if (!exists) {
        // Create new page directory pointer table
        uintptr_t p_address = (uintptr_t)&bootstrap_pdp[bootstrap_pdp_index];
        bootstrap_pdp_index++;
        page_directory_pointer_table* target_pdp = &pdp_tables[l4_index];
        map_page_to(p_address, (uintptr_t)target_pdp);
        for (int i = 0; i < 512; i++) { target_pdp->data[i] = 0; }
        pml4_table->data[l4_index]
            = (uintptr_t)p_address | pdp_present | pdp_write_enabled;
    }

    return &pdp_tables[l4_index];
};

inline page_directory_table* get_page_directory(uintptr_t virtual_address) {

    page_directory_pointer_table* parent_table
        = get_page_directory_pointer_table(virtual_address);

    // Check if it has a matching PD entry in the PDP table
    int master_index
        = ((virtual_address % page_level_4_table_size) / page_directory_size);
    virtual_address %= page_directory_pointer_size;
    int  parent_index = (virtual_address / page_directory_size);
    bool exists       = (parent_table->data[parent_index] != 0);

    if (!exists) {
        // Create new page directory table
        uintptr_t p_address = (uintptr_t)&bootstrap_pd[bootstrap_pd_index];
        bootstrap_pd_index++;
        page_directory_table* target_pd = &pd_tables[master_index];
        map_page_to(p_address, (uintptr_t)target_pd);
        for (int i = 0; i < 512; i++) { target_pd->data[i] = 0; }
        parent_table->data[parent_index]
            = (uintptr_t)p_address | pd_present | pd_write_enabled;
    }

    return &pd_tables[master_index];
};

inline page_table* get_page_table(uintptr_t virtual_address) {

    page_directory_table* parent_table = get_page_directory(virtual_address);

    // Check if it has a matching PT entry in the PD table
    int master_index
        = ((virtual_address % page_level_4_table_size) / page_table_size);
    virtual_address %= page_directory_size;
    int  parent_index = (virtual_address / page_table_size);
    bool exists       = (parent_table->data[parent_index] != 0);

    if (!exists) {
        // Create new page table
        uintptr_t p_address = (uintptr_t)&bootstrap_pt[bootstrap_pt_index];
        bootstrap_pt_index++;
        page_table* target_pt = &pt_tables[master_index];
        map_page_to(p_address, (uintptr_t)target_pt);
        for (int i = 0; i < 512; i++) { target_pt->data[i] = 0; }
        parent_table->data[parent_index]
            = (uintptr_t)p_address | pt_present | pt_write_enabled;
    }

    return &pt_tables[master_index];
};

inline page_entry* get_page(uintptr_t virtual_address) {

    page_table* target_pt = get_page_table(virtual_address);

    // Return the corresponding page entry
    virtual_address %= page_table_size;
    return &(target_pt->data[virtual_address / PAGE_SIZE]);
};

inline void* virt_to_phys(uintptr_t virtual_address) {
    return (void*)(*get_page((virtual_address)) & real_address_bitmask);
};

void map_page_to(uintptr_t source_address, uintptr_t target_address) {
    *(get_page(target_address)) = ((source_address & real_address_bitmask)
                                   | p_present | p_write_enabled);
    refresh_page((void*)target_address);
}

void map_pt_to(uintptr_t source_address, uintptr_t target_address) {
    page_table* target_table = get_page_table(target_address);

    // Loop through entries of the target table
    source_address = (source_address - (source_address % page_table_size))
                     | p_present | p_write_enabled;
    for (int i = 0; i < 512; i++) {
        target_table->data[i] = source_address;
        source_address += PAGE_SIZE;
        refresh_page((void*)target_address);
    }
}

void map_pd_to(uintptr_t source_address, uintptr_t target_address) {
    page_directory_table* target_directory = get_page_directory(target_address);

    // Loop through the page tables
    source_address = (source_address - (source_address % page_directory_size))
                     | p_present | p_write_enabled;
    for (int i = 0; i < 512; i++) {
        page_table* target_table = (page_table*)target_directory->data[i];

        // Check if this table exists
        if (target_table == 0) {
            // Doesn't exist, need to create it first
            uintptr_t p_address = (uintptr_t)&bootstrap_pt[bootstrap_pt_index];
            bootstrap_pt_index++;
            target_table = &pt_tables[(target_address / page_table_size)];
            map_page_to(p_address, (uintptr_t)target_table);
            for (int i = 0; i < 512; i++) { target_table->data[i] = 0; }

            target_directory->data[i]
                = p_address | pt_present | pt_write_enabled;
        }

        // Map the entries of this table
        for (int i = 0; i < 512; i++) {
            target_table->data[i] = source_address;
            source_address += PAGE_SIZE;
            refresh_page((void*)target_address);
        }
    }
}

void map_region_to(uintptr_t source_address, uintptr_t target_address,
                   size_t size) {

    // Ensure that addresses (and size) fall along page lines
    target_address &= ~(PAGE_SIZE - 1);
    source_address &= ~(PAGE_SIZE - 1);
    if (size & (PAGE_SIZE - 1)) { size += PAGE_SIZE; }
    size &= ~(PAGE_SIZE - 1);

    while (size) {
        // Break off into different levels of mapping
        if ((size > page_directory_size)
            && ((target_address & (page_directory_size - 1)) == 0)) {
            // Map next page directory
            map_pd_to(source_address, target_address);
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
            map_pt_to(source_address, target_address);
            source_address += page_table_size;
            target_address += page_table_size;
            if (size > page_table_size) {
                size -= page_table_size;
            } else {
                size = 0;
            }

        } else {
            // Map next page
            map_page_to(source_address, target_address);
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
namespace paging {

#ifndef BSS_SIZE
    #define BSS_SIZE 0
#endif

// Region of memory reserved for kernel's BSS section
unsigned char kernel_bss[BSS_SIZE] __attribute__((aligned(PAGE_SIZE)));
unsigned int  kernel_bss_index = 0;

struct initial_mapping {
    page_directory_pointer_table pdp[2];
    page_directory_table         pd[4];
    page_table                   pt[8];

    void prepare_table_mapping(page_level_4_table* level_4_table) {

        // Prepare the initial tables
        // PML4T
        level_4_table->data[0x1fe]
            = (((uintptr_t)&pdp[0] & real_address_bitmask) | pd_present
               | pd_write_enabled);
        level_4_table->data[0x1ff]
            = (((uintptr_t)&pdp[1] & real_address_bitmask) | pd_present
               | pd_write_enabled);

        // PDPs
        pdp[0].data[0x1fe] = (((uintptr_t)&pd[0] & real_address_bitmask)
                              | pd_present | pd_write_enabled);
        pdp[0].data[0x1ff] = (((uintptr_t)&pd[1] & real_address_bitmask)
                              | pd_present | pd_write_enabled);

        pdp[1].data[0x1fe] = (((uintptr_t)&pd[2] & real_address_bitmask)
                              | pd_present | pd_write_enabled);
        pdp[1].data[0x1ff] = (((uintptr_t)&pd[3] & real_address_bitmask)
                              | pd_present | pd_write_enabled);

        // PDs
        pd[0].data[0x1fe] = (((uintptr_t)&pt[0] & real_address_bitmask)
                             | pt_present | pt_write_enabled);
        pd[0].data[0x1ff] = (((uintptr_t)&pt[1] & real_address_bitmask)
                             | pt_present | pt_write_enabled);

        pd[1].data[0x1fe] = (((uintptr_t)&pt[2] & real_address_bitmask)
                             | pt_present | pt_write_enabled);
        pd[1].data[0x1ff] = (((uintptr_t)&pt[3] & real_address_bitmask)
                             | pt_present | pt_write_enabled);

        pd[2].data[0x1fe] = (((uintptr_t)&pt[4] & real_address_bitmask)
                             | pt_present | pt_write_enabled);
        pd[2].data[0x1ff] = (((uintptr_t)&pt[5] & real_address_bitmask)
                             | pt_present | pt_write_enabled);

        pd[3].data[0x1fe] = (((uintptr_t)&pt[6] & real_address_bitmask)
                             | pt_present | pt_write_enabled);
        pd[3].data[0x1ff] = (((uintptr_t)&pt[7] & real_address_bitmask)
                             | pt_present | pt_write_enabled);

        // Mappings in PTs
        // PML4T
        pt[0].data[((uintptr_t)pml4_table % page_table_size) / PAGE_SIZE]
            = (((uintptr_t)level_4_table & real_address_bitmask) | p_present
               | p_write_enabled);

        // PDPs
        pt[1]
            .data[((uintptr_t)&pdp_tables[0x1fe] % page_table_size) / PAGE_SIZE]
            = (((uintptr_t)&pdp[0] & real_address_bitmask) | p_present
               | p_write_enabled);
        pt[1]
            .data[((uintptr_t)&pdp_tables[0x1ff] % page_table_size) / PAGE_SIZE]
            = (((uintptr_t)&pdp[1] & real_address_bitmask) | p_present
               | p_write_enabled);

        // PDs
        pt[2].data[((uintptr_t)&pd_tables[(0x200 * 0x1fe) + 0x1fe]
                    % page_table_size)
                   / PAGE_SIZE]
            = (((uintptr_t)&pd[0] & real_address_bitmask) | p_present
               | p_write_enabled);
        pt[2].data[((uintptr_t)&pd_tables[(0x200 * 0x1fe) + 0x1ff]
                    % page_table_size)
                   / PAGE_SIZE]
            = (((uintptr_t)&pd[1] & real_address_bitmask) | p_present
               | p_write_enabled);

        pt[3].data[((uintptr_t)&pd_tables[(0x200 * 0x1ff) + 0x1fe]
                    % page_table_size)
                   / PAGE_SIZE]
            = (((uintptr_t)&pd[2] & real_address_bitmask) | p_present
               | p_write_enabled);
        pt[3].data[((uintptr_t)&pd_tables[(0x200 * 0x1ff) + 0x1ff]
                    % page_table_size)
                   / PAGE_SIZE]
            = (((uintptr_t)&pd[3] & real_address_bitmask) | p_present
               | p_write_enabled);

        // PTs
        pt[4].data[((uintptr_t)&pt_tables[(0x40000 * 0x1fe) + (0x200 * 0x1fe)
                                          + 0x1fe]
                    % page_table_size)
                   / PAGE_SIZE]
            = (((uintptr_t)&pt[0] & real_address_bitmask) | p_present
               | p_write_enabled);
        pt[4].data[((uintptr_t)&pt_tables[(0x40000 * 0x1fe) + (0x200 * 0x1fe)
                                          + 0x1ff]
                    % page_table_size)
                   / PAGE_SIZE]
            = (((uintptr_t)&pt[1] & real_address_bitmask) | p_present
               | p_write_enabled);

        pt[5].data[((uintptr_t)&pt_tables[(0x40000 * 0x1fe) + (0x200 * 0x1ff)
                                          + 0x1fe]
                    % page_table_size)
                   / PAGE_SIZE]
            = (((uintptr_t)&pt[2] & real_address_bitmask) | p_present
               | p_write_enabled);
        pt[5].data[((uintptr_t)&pt_tables[(0x40000 * 0x1fe) + (0x200 * 0x1ff)
                                          + 0x1ff]
                    % page_table_size)
                   / PAGE_SIZE]
            = (((uintptr_t)&pt[3] & real_address_bitmask) | p_present
               | p_write_enabled);

        pt[6].data[((uintptr_t)&pt_tables[(0x40000 * 0x1ff) + (0x200 * 0x1fe)
                                          + 0x1fe]
                    % page_table_size)
                   / PAGE_SIZE]
            = (((uintptr_t)&pt[4] & real_address_bitmask) | p_present
               | p_write_enabled);
        pt[6].data[((uintptr_t)&pt_tables[(0x40000 * 0x1ff) + (0x200 * 0x1fe)
                                          + 0x1ff]
                    % page_table_size)
                   / PAGE_SIZE]
            = (((uintptr_t)&pt[5] & real_address_bitmask) | p_present
               | p_write_enabled);

        pt[7].data[((uintptr_t)&pt_tables[(0x40000 * 0x1ff) + (0x200 * 0x1ff)
                                          + 0x1fe]
                    % page_table_size)
                   / PAGE_SIZE]
            = (((uintptr_t)&pt[6] & real_address_bitmask) | p_present
               | p_write_enabled);
        pt[7].data[((uintptr_t)&pt_tables[(0x40000 * 0x1ff) + (0x200 * 0x1ff)
                                          + 0x1ff]
                    % page_table_size)
                   / PAGE_SIZE]
            = (((uintptr_t)&pt[7] & real_address_bitmask) | p_present
               | p_write_enabled);

        // Refresh all tables
        refresh_all_pages();
    }
} initial_mapping __attribute__((aligned(PAGE_SIZE)));
} // namespace paging

// Multiboot Header + tags
struct mb_header {
    multiboot_header header
        = {MULTIBOOT2_HEADER_MAGIC, MULTIBOOT_ARCHITECTURE_I386,
           (uint32_t)sizeof(mb_header),
           -(uint32_t)(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386
                       + (uint32_t)sizeof(mb_header))};

    struct tag_request_header {
        multiboot_header_tag_information_request tag_request
            = {MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST,
               0,
               (uint32_t)sizeof(tag_request_header),
               {}};

        multiboot_uint32_t tags[7] = {
            MULTIBOOT_TAG_TYPE_BASIC_MEMINFO, MULTIBOOT_TAG_TYPE_ACPI_NEW,
            MULTIBOOT_TAG_TYPE_EFI64,         MULTIBOOT_TAG_TYPE_FRAMEBUFFER,
            MULTIBOOT_TAG_TYPE_MMAP,          MULTIBOOT_TAG_TYPE_EFI_MMAP,
            MULTIBOOT_TAG_TYPE_MODULE,
        };
    } tag_request_header __attribute__((aligned(8)));

    multiboot_header_tag_framebuffer framebuffer __attribute__((aligned(8)))
    = {MULTIBOOT_HEADER_TAG_FRAMEBUFFER,
       0,
       (uint32_t)sizeof(multiboot_header_tag_framebuffer),
       FB_DEFAULT_WIDTH,
       FB_DEFAULT_HEIGHT,
       FB_DEFAULT_DEPTH};

    multiboot_header_tag terminator_tag __attribute__((aligned(8))) = {0, 0, 8};
} mb_header __attribute__((aligned(MULTIBOOT_HEADER_ALIGN)))
__attribute__((section(".multiboot")));

void* memcpy(void* __restrict__ dest_ptr, const void* __restrict__ src_ptr,
             size_t size) {

    for (unsigned int i = 0; i < size; i++) {

        ((unsigned char*)dest_ptr)[i] = ((unsigned char*)src_ptr)[i];
    }

    return dest_ptr;
}

void mem_bss(void* __restrict__ dest_ptr, size_t size) {

    for (unsigned int i = 0; i < size; i++) {

        ((unsigned char*)dest_ptr)[i] = 0;
    }
}

void virtual_load(elf_64::file_header* elf_file) {

    elf_64::section_header* file_section
        = (elf_64::section_header*)((uintptr_t)elf_file
                                    + (uintptr_t)(elf_file->e_shoff
                                                  & 0xffffffff));

    size_t entry_size = elf_file->e_shentsize;

    for (uint16_t i = 0; i < elf_file->e_shnum; i++) {

        if (!(file_section->sh_addr == 0)) {

            // Calculate start address
            void* source = (void*)((uintptr_t)elf_file
                                   + (file_section->sh_offset & 0xffffffff));

            // Load the section into memory
            if (file_section->sh_type == 0x8) {
                // BSS section needs mapped from reserved section
                paging::map_region_to(
                    (uintptr_t)&paging::kernel_bss[paging::kernel_bss_index],
                    (uintptr_t)file_section->sh_addr,
                    (size_t)file_section->sh_size);

                // Increase kernel bss index
                paging::kernel_bss_index += (unsigned int)file_section->sh_size;

                // BSS segment should be zeroed out
                mem_bss((void*)file_section->sh_addr,
                        (size_t)file_section->sh_size);
            } else {
                paging::map_region_to((uintptr_t)source,
                                      (uintptr_t)file_section->sh_addr,
                                      (size_t)file_section->sh_size);
            }
        }

        // Move to the next section
        file_section
            = (elf_64::section_header*)((uintptr_t)file_section + entry_size);
    }
}

extern "C" {

extern paging::page_level_4_table           PML4T;
extern paging::page_directory_pointer_table PDPT;
extern paging::page_directory_table         PDT;
extern paging::page_table                   PT[2];

void prep_entry(uint64_t* k_entry, const multiboot_boot_info* mb_info) {

    // Prepare initial mappings
    paging::initial_mapping.prepare_table_mapping(&PML4T);
    paging::map_page_to((uintptr_t)&PDPT, (uintptr_t)&paging::pdp_tables[0]);
    paging::map_page_to((uintptr_t)&PDT, (uintptr_t)&paging::pd_tables[0]);
    paging::map_page_to((uintptr_t)&PT[0], (uintptr_t)&paging::pt_tables[0]);
    paging::map_page_to((uintptr_t)&PT[1], (uintptr_t)&paging::pt_tables[1]);

    if ((uintptr_t)mb_info->kernel_module_tag) {

        // Find the kernel entry point
        uint64_t* flat_entry
            = (uint64_t*)(&((elf_64::
                                 file_header*)((((multiboot_tag_module*)((uintptr_t)mb_info
                                                                             ->kernel_module_tag
                                                                         & 0xffffffff))
                                                    ->mod_start)
                                               | 0UL))
                               ->e_entry);
        *k_entry = *flat_entry;

        // Load the kernel module into the correct position in memory
        virtual_load(
            (elf_64::
                 file_header*)((((multiboot_tag_module*)((uintptr_t)mb_info
                                                             ->kernel_module_tag
                                                         & 0xffffffff))
                                    ->mod_start)
                               | 0UL));
    }
}
}

extern "C" {
void send_init_serial() {

    const char string[] = "PintOS Loading...\r\n";

    int i = 0;
    while (1) {

        // Get next char
        char data = string[i];

        // Check for null termination
        if (data == '\0') {
            break;
        } else {
            asm volatile("outb %%al, %%dx"
                         :
                         : "a"(data | 0UL), "d"(0x3f8 | 0UL)
                         :);
            i++;
        }
    }
}
}

bool string_compare(const char* first, const char* second) {

    int i = 0;
    while (1) {
        if (first[i] == '\0' && second[i] == '\0') {
            return true;
        } else if (first[i] != second[i]) {
            return false;
        } else {
            i++;
        }
    }
}

extern "C" {

void process_tags(multiboot_boot_info* destination, multiboot_tag* source) {

    // Save origin info
    destination->mb_start = (void*)((uintptr_t)source | 0ULL);
    destination->mb_size  = *((uint32_t*)source);

    // Save thread startup code info
    destination->thread_start = (void*)((uintptr_t)&thread_startup | 0ULL);
    destination->thread_stack_top
        = (void**)((uintptr_t)&next_thread_stack_top | 0ULL);
    destination->thread_target
        = (void**)((uintptr_t)&thread_startup_target_code | 0ULL);
    destination->thread_size
        = (size_t)((uintptr_t)&thread_startup_end - (uintptr_t)&thread_startup);

    // Get total size and move to the first tag
    uintptr_t tags_end = ((uintptr_t)source + *((uint32_t*)source));
    source             = (multiboot_tag*)((uintptr_t)source + 8);

    size_t info_size = sizeof(multiboot_boot_info);

    while ((uintptr_t)source < tags_end) {

        if (source->type == MULTIBOOT_TAG_TYPE_MODULE
            && !(string_compare(
                (const char*)(&(((multiboot_tag_module*)source)->cmdline)),
                kernel_mod_identifier))) {
            // Extra module tag
            destination->extra_module[destination->num_extra_modules]
                = (multiboot_tag_module*)((uintptr_t)source | 0);
            destination->num_extra_modules++;
            info_size += sizeof(void*);

        } else {
            destination->tag[source->type]
                = (multiboot_tag*)((uintptr_t)source | 0);
        }

        // Move to next tag (note all tags are 8-byte aligned, and this
        // alignment is not included in the given size parameter)
        source = (multiboot_tag*)((uintptr_t)source + source->size);
        if ((uintptr_t)source & 0x7) {
            source = (multiboot_tag*)((uintptr_t)source
                                      + (8 - ((uintptr_t)source & 0x7)));
        }
    }

    // Save total size
    destination->total_size = info_size;
}
}

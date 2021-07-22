/**
 * @file loader.cpp
 * @author Shane Menzies
 * @brief Boot loader to take over and prepare the boot environment before 
 *        handing over to the kernel
 * @date 05/21/21
 * 
 * 
 */

#include "loader.h"

// Multiboot Header + tags
struct mb_header {
    multiboot_header header = {
        MULTIBOOT2_HEADER_MAGIC, 
        MULTIBOOT_ARCHITECTURE_I386, 
        sizeof(mb_header), 
        -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + sizeof(mb_header)) 
    };

    struct tag_request_header {
        multiboot_header_tag_information_request tag_request = {
            MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST,
            0,
            sizeof(tag_request_header),
            {}
        };

        multiboot_uint32_t tags[7] = {
            MULTIBOOT_TAG_TYPE_BASIC_MEMINFO,
            MULTIBOOT_TAG_TYPE_ACPI_NEW,
            MULTIBOOT_TAG_TYPE_EFI64,
            MULTIBOOT_TAG_TYPE_FRAMEBUFFER,
            MULTIBOOT_TAG_TYPE_MMAP,
            MULTIBOOT_TAG_TYPE_EFI_MMAP,
            MULTIBOOT_TAG_TYPE_MODULE,
        };
    } tag_request_header __attribute__ ((aligned(8)));

    multiboot_header_tag_framebuffer framebuffer __attribute__ ((aligned(8))) = {
        MULTIBOOT_HEADER_TAG_FRAMEBUFFER,
        0,
        sizeof(multiboot_header_tag_framebuffer),
        FB_DEFAULT_WIDTH,
        FB_DEFAULT_HEIGHT,
        FB_DEFAULT_DEPTH
    };

    multiboot_header_tag terminator_tag __attribute__ ((aligned(8))) = {0, 0, 8};
} mb_header __attribute__ ((aligned(MULTIBOOT_HEADER_ALIGN))) __attribute__ ((section(".multiboot")));

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
            asm volatile (
            "outb %%al, %%dx"
            :
            : "a" (data | 0UL), "d" (0x3f8 | 0UL)
            :
            );
            i++;
        }
    }
}}

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

void direct_load(elf_64::file_header* elf_file) {

    elf_64::section_header* file_section = (elf_64::section_header*)((uintptr_t)elf_file 
                                          + (uintptr_t)(elf_file->e_shoff & 0xffffffff));
    size_t header_size = elf_file->e_shentsize;

    for (uint16_t i = 0; i < elf_file->e_shnum; i++) {
        
        if (!(file_section->sh_addr == 0)) {

            // Calculate start address
            void* source = (void*)((uintptr_t)elf_file + (file_section->sh_offset & 0xffffffff));

            // Load the section into memory
            if (file_section->sh_type == 0x8) {
                // BSS segment should be zeroed out
                mem_bss((void*)file_section->sh_addr, (size_t)file_section->sh_size);
            } else {
                memcpy((void*)file_section->sh_addr, source, (size_t)file_section->sh_size);
            }
        }

        // Move to the next section
        file_section = (elf_64::section_header*)((uintptr_t)file_section + header_size);
    }
}

extern "C" {

void process_tags(multiboot_boot_info* destination, multiboot_tag* source) {

    // Save origin info
    destination->mb_start = (uint64_t)((uintptr_t)source | 0ULL);
    destination->mb_size = *((uint32_t*)source);

    // Get total size and move to the first tag
    uintptr_t tags_end = ((uintptr_t)source + *((uint32_t*)source));
    source = (multiboot_tag*)((uintptr_t)source + 8);

    size_t info_size = sizeof(multiboot_boot_info);

    while ((uintptr_t)source < tags_end) {

        if (source->type == MULTIBOOT_TAG_TYPE_MODULE && !(string_compare((const char*)(&(((multiboot_tag_module*)source)->cmdline)), kernel_mod_identifier))) {
            // Extra module tag
            destination->extra_module[destination->num_extra_modules] = (uint64_t)((uintptr_t)source | 0);
            destination->num_extra_modules++;
            info_size += sizeof(void*);

        } else {
            destination->tag[source->type] = (uint64_t)((uintptr_t)source | 0);
        }

        // Move to next tag (note all tags are 8-byte aligned, and this alignment is not included in the given size parameter)
        source = (multiboot_tag*)((uintptr_t)source + source->size);
        if ((uintptr_t)source & 0x7) {
            source = (multiboot_tag*)((uintptr_t)source + (8 - ((uintptr_t)source & 0x7)));
        }
    }

    // Save total size
    destination->total_size = info_size;
}

void prep_64(uint32_t* k_entry, const multiboot_boot_info* mb_info) {

    if ((uintptr_t)mb_info->kernel_module_tag) {
        
        // Find the kernel entry point
        uint32_t* flat_entry = (uint32_t*)(&((elf_64::file_header*)((multiboot_tag_module*)((uint64_t)mb_info->kernel_module_tag & 0xffffffff))->mod_start)->e_entry);
        k_entry[0] = flat_entry[0];
        k_entry[1] = flat_entry[1];

        // Load the kernel module into the correct position in memory
        direct_load(((elf_64::file_header*)((multiboot_tag_module*)((uint64_t)mb_info->kernel_module_tag & 0xffffffff))->mod_start));

    }

}

}


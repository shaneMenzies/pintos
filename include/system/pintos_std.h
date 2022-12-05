#ifndef PINTOS_STD
#define PINTOS_STD

#include "system/multiboot.h"
#include "system/stack_check.h"

#include <stddef.h>
#include <stdint.h>

enum efi_mem_type : uint32_t {
    EfiReservedMemoryType,      // Not Usable
    EfiLoaderCode,              // Usable
    EfiLoaderData,              // Usable
    EfiBootServicesCode,        // Usable
    EfiBootServicesData,        // Usable
    EfiRuntimeServicesCode,     // Not Usable
    EfiRuntimeServicesData,     // Not Usable
    EfiConventionalMemory,      // Usable
    EfiUnusableMemory,          // Not Usable
    EfiACPIReclaimMemory,       // Special ACPI Reclaimable
    EfiACPIMemoryNVS,           // Not Usable
    EfiMemoryMappedIO,          // Not Usable
    EfiMemoryMappedIOPortSpace, // Not Usable
    EfiPalCode,                 // Not Usable
    EfiPersistentMemory,        // Usable
    EfiUnacceptedMemoryType,    // Not Usable
    EfiMaxMemoryType            // Not Usable

    // Usable: 1-4, 7, 14
    // ACPI Reclaimable: 9
    // Unusable: 0, 5, 6, 8-13, 15+
};

#define EFI_PAGE_SIZE 0x1000

#if __x86_64__

struct efi_mmap_entry {
    efi_mem_type type;
    void*        p_start;
    void*        v_start;
    uint64_t     num_pages;
    uint64_t     attribute;
};

struct multiboot_boot_info {

    void*  boot_start;
    size_t boot_size;

    void* stack_bottom;
    void* stack_top;

    void*  thread_start;
    size_t thread_size;
    void** thread_target;
    void** thread_stack_top;

    void*  mb_start;
    size_t mb_size;

    // Total size of this entire boot info struct
    size_t total_size;

    // Possible tags that multiboot can give us, also accessible as an array,
    // where the index matches the type value for that tag
    // ex: (tag[MULTIBOOT_TAG_TYPE_MMAP] == tag[6] == mmap_tag)
    union {
        struct {
            multiboot_tag*                end_tag;
            multiboot_tag_string*         cmdline_tag;
            multiboot_tag_string*         loader_tag;
            multiboot_tag_module*         kernel_module_tag;
            multiboot_tag_basic_meminfo*  meminfo_tag;
            multiboot_tag_bootdev*        bootdev_tag;
            multiboot_tag_mmap*           mmap_tag;
            multiboot_tag_vbe*            vbe_tag;
            multiboot_tag_framebuffer*    framebuffer_tag;
            multiboot_tag_elf_sections*   elf_sect_tag;
            multiboot_tag_apm*            apm_tag;
            multiboot_tag_efi32*          efi32_tag;
            multiboot_tag_efi64*          efi64_tag;
            multiboot_tag_smbios*         smbios_tag;
            multiboot_tag_old_acpi*       acpi_old_tag;
            multiboot_tag_new_acpi*       acpi_new_tag;
            multiboot_tag_network*        network_tag;
            multiboot_tag_efi_mmap*       efi_mmap_tag;
            multiboot_tag*                bootserv_tag;
            multiboot_tag_efi32_ih        efi32_ih_tag;
            multiboot_tag_efi64_ih        efi64_ih_tag;
            multiboot_tag_load_base_addr* load_base_addr_tag;
        };
        multiboot_tag* tag[22] = {0};
    };

    // Extra space for additional module tags being provided
    uint64_t              num_extra_modules = 0;
    multiboot_tag_module* extra_module[];
};

#else

// x86 versions to keep compatibility with 64-bit mode

struct efi_mmap_entry {
    efi_mem_type type;
    uint64_t     p_start;
    uint64_t     v_start;
    uint64_t     num_pages;
    uint64_t     attribute;
};

struct multiboot_boot_info {

    uint64_t boot_start;
    uint64_t boot_size;

    uint64_t stack_bottom;
    uint64_t stack_top;

    uint64_t thread_start;
    uint64_t thread_size;
    uint64_t thread_target;
    uint64_t thread_stack_top;

    uint64_t mb_start;
    uint64_t mb_size;

    // Total size of this entire boot info struct
    uint64_t total_size;

    // Possible tags that multiboot can give us, also accessible as an array,
    // where the index matches the type value for that tag
    // ex: (tag[MULTIBOOT_TAG_TYPE_MMAP] == tag[6] == mmap_tag)
    union {
        struct {
            uint64_t end_tag;
            uint64_t cmdline_tag;
            uint64_t loader_tag;
            uint64_t kernel_module_tag;
            uint64_t meminfo_tag;
            uint64_t bootdev_tag;
            uint64_t mmap_tag;
            uint64_t vbe_tag;
            uint64_t framebuffer_tag;
            uint64_t elf_sect_tag;
            uint64_t apm_tag;
            uint64_t efi32_tag;
            uint64_t efi64_tag;
            uint64_t smbios_tag;
            uint64_t acpi_old_tag;
            uint64_t acpi_new_tag;
            uint64_t network_tag;
            uint64_t efi_mmap_tag;
            uint64_t bootserv_tag;
            uint64_t efi32_ih_tag;
            uint64_t efi64_ih_tag;
            uint64_t load_base_addr_tag;
        };
        uint64_t tag[22] = {0};
    };

    // Extra space for additional module tags being provided
    uint64_t num_extra_modules = 0;
    uint64_t extra_module[];
};

#endif

#endif
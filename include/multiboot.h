#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>
#include <stdbool.h>

extern bool mb_valid;

extern struct mb_info multiboot;

/**
 * @brief Constants for the multiboot flags meaning
 * 
 */
enum mb_info_flags {
    MEM_FLAG = 1<<0,
    BOOT_DEVICE_FLAG = 1<<1,
    CMDLINE_FLAG = 1<<2,
    MODS_FLAG = 1<<3,
    SYMS_FLAG = 1<<4 | 1<<5,
    MMAP_FLAG = 1<<6,
    DRIVES_FLAG = 1<<7,
    CONFIG_FLAG = 1<<8,
    BOOT_LD_NAME_FLAG = 1<<9,
    APM_FLAG = 1<<10,
    VBE_FLAG = 1<<11,
    FRAMEBUFFER_FLAG = 1<<12, 
};

/**
 * @brief Structure for the info left in memory by multiboot
 * 
 */
struct mb_info {

    uint32_t flags;             // Always present

    uint32_t mem_lower;         // Present with flags[0]
    uint32_t mem_upper;

    uint32_t boot_device;       // Present with flags[1]

    uint32_t cmdline;           // Present with flags[2]

    uint32_t mods_count;        // Present with flags[3]
    uint32_t mods_addr;

    uint32_t syms[4];           // Present with either flags[4] or flags[5]

    uint32_t mmap_length;       // Present with flags[6]
    uint32_t mmap_addr;

    uint32_t drives_length;     // Present with flags[7]
    uint32_t drives_addr;

    uint32_t config_table;      // Present with flags[8]

    uint32_t boot_loader_name;  // Present with flags[9]

    uint32_t apm_table;         // Present with flags[10]

    uint32_t vbe_control_info;  // Present with flags[11]
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

    uint64_t framebuffer_addr;  // Present with flags[12]
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;    // Bits per pixel
    uint8_t framebuffer_type;
    uint8_t color_info[6];
} __attribute__((packed));

#endif
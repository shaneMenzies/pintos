/**
 * @file gdt.cpp
 * @author Shane Menzies
 * @brief Management of the Global Descriptor Table
 * @date 03/15/21
 * 
 * 
 */

#include "gdt.h"

extern "C" {
    void set_gdt(uint32_t gdt_addr, uint16_t gdt_size);
    void reload_gdt();
}

x86_tables::gdt_segment gdt_table[3];

/**
 * @brief Initializes the Global Descriptor Table
 * 
 */
void gdt_init() {

    // Null selector
    x86_tables::set_segment(gdt_table[0], 0x0, (void*)0, 0, 0);

    // Code segment
    x86_tables::set_segment(gdt_table[1], 0xffffffff, (void*)0, 0x9a, 0b0101);

    // Data segment
    x86_tables::set_segment(gdt_table[2], 0xffffffff, (void*)0, 0x92, 0b0101);

    size_t total_table_size = sizeof(x86_tables::gdt_segment) * 3;

    set_gdt((uint32_t)gdt_table, (uint16_t)total_table_size);
    reload_gdt();
}

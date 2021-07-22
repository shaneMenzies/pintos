/**
 * @file x86_tables.cpp
 * @author Shane Menzies
 * @brief Various tables necessary for x86 processors
 * @date 03/11/21
 * 
 * 
 */

#include "x86_tables.h"

namespace x86_tables {

    

    struct gdt_table_structure {
        gdt_segment null;
        gdt_segment code;
        gdt_segment data;
    } gdt_table;

    /*
    void gdt_init() {

        // Prepare the gdt table
        set_segment(gdt_table.null, 0, 0, null_access, null_flags);
        set_segment(gdt_table.code, 0, 0xffffffff, code_access, long_paged);
        set_segment(gdt_table.data, 0, 0xffffffff, data_access, long_paged);

        uint16_t gdt_size = sizeof(gdt_table) - 1;

        //set_gdt(&gdt_table, gdt_size);
    }
    */

    /**
     * @brief Get the offset of the target IDT Gate
     * 
     * @param target    Target IDT Gate
     * @return void*    Offset of the target gate
     */
    void* get_offset(const idt_gate target) {
        return (void*)(((uint64_t)target.offset_3 << 32) | ((uint64_t)target.offset_2 << 16) | (uint64_t)target.offset_1);
    }

    /**
     * @brief Set the offset of the target IDT Gate
     * 
     * @param target        Target IDT Gate
     * @param new_offset    New offset for the target gate
     */
    void set_offset(idt_gate& target, void* new_offset) {
        target.offset_1 = (uint16_t)((uintptr_t)new_offset & 0xffff);
        target.offset_2 = (uint16_t)(((uintptr_t)new_offset >> 16) & 0xffff);
    }

    /**
     * @brief Set all of the values of a certain IDT gate descriptor
     * 
     * @param target        IDT Gate to be modified
     * @param offset        Offset of interrupt handler
     * @param gdt_selector  GDT Index of a segment selector
     * @param min_privilege Minimum privilege level for the handler
     * @param type          Type field
     * @param present       Whether this gate is valid or not
     * @param task_gate     Whether this is a task gate or not
     */
    void set_gate(idt_gate& target, void* offset, gdt_selector selector, 
            uint8_t min_privilege, uint8_t type, bool present) {

        uint8_t type_top = (((min_privilege << 1) & 0x6) | (present ? 8 : 0)) << 4;
        target.offset_1 = (uint16_t)((uintptr_t)offset & 0xffff);
        target.offset_2 = (uint16_t)(((uintptr_t)offset >> 16) & 0xffff);
        target.offset_3 = (uint32_t)(((uintptr_t)offset >> 32) & 0xffffffff);

        target.selector = selector;

        target.type_attr = ((type & 0xf) | (type_top & 0xf0));
    }

    void set_segment(gdt_segment& target, void* base, uint32_t limit,
            gdt_access_types access, gdt_flags flags) {

        target.base_1 = ((uintptr_t)base & 0xffff);
        base = (void*)((uintptr_t)base >> 16);
        target.base_2 = ((uintptr_t)base & 0xff);
        base = (void*)((uintptr_t)base >> 8);
        target.base_3 = ((uintptr_t)base & 0xff);
        base = (void*)((uintptr_t)base >> 8);
        target.base_4 = ((uintptr_t)base & 0xffffffff);

        target.limit_1 = (limit & 0xffff);
        limit >>= 16;
        target.flags_limit_2 = (limit & 0x0f);

        target.flags_limit_2 |= flags;
        target.access = access;
    }
}

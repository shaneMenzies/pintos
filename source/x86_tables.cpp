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

    void* get_offset(idt_gate target) {
        return (void*)((target.offset_2 << 16) | target.offset_1);
    }

    void set_offset(idt_gate& target, void* new_offset) {
        target.offset_1 = (uint16_t)((uintptr_t)new_offset & 0xffff);
        target.offset_2 = (uint16_t)(((uintptr_t)new_offset >> 16) & 0xffff);
    }

    void set_segment(gdt_segment& target, uint32_t limit, void* base, 
                     uint8_t type, uint8_t flags) {

        target.limit_1 = (uint16_t)(limit & 0xffff);
        limit >>= 16;
        target.limit_2_flags &= (uint8_t)(limit & 0x0f);
        target.limit_2_flags |= ((flags & 0x0f) << 4);

        target.base_1 = (uint16_t)((uintptr_t)base & 0xffff);
        base = (void*)((uintptr_t)base >> 16);
        target.base_2 = (uint8_t)((uintptr_t)base & 0xff);
        base = (void*)((uintptr_t)base >> 8);
        target.base_3 = (uint8_t)((uintptr_t)base & 0xff);

        target.type_byte = type;
    }

    void set_gate(idt_gate& target, void* offset, uint16_t gdt_selector, 
            uint8_t min_privilege, uint8_t type, bool present, bool task_gate) {

        uint8_t type_top = ((task_gate ? 1 : 0) | ((min_privilege << 1) & 0x6) 
                            | (present ? 8 : 0)) << 4;
        target.offset_1 = (uint16_t)((uintptr_t)offset & 0xffff);
        target.offset_2 = (uint16_t)(((uintptr_t)offset >> 16) & 0xffff);

        target.selector = gdt_selector;

        target.type_attr = ((type & 0xf) | (type_top & 0xf0));
    }
}

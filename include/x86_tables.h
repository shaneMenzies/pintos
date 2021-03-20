#ifndef TABLES_H
#define TABLES_H

#include <stdint.h>

namespace x86_tables {
    
    #pragma pack(push , 0)
    struct idt_gate {
        uint16_t offset_1;
        uint16_t selector;
        uint8_t zero = 0;
        uint8_t type_attr;
        uint16_t offset_2;
    };

    struct gdt_segment {
        uint16_t limit_1;
        uint16_t base_1;
        uint8_t base_2;
        uint8_t type_byte;
        uint8_t limit_2_flags;
        uint8_t base_3;
    };
    #pragma pack(pop)

    void* get_offset(idt_gate target);

    void set_gate(idt_gate& target, void* offset, uint16_t gdt_selector, 
             uint8_t min_privilege, uint8_t type, bool present, bool task_gate);

    void set_segment(gdt_segment& target, uint32_t limit, void* base, 
                    uint8_t type, uint8_t flags);
}

#endif
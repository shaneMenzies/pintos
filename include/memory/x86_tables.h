#ifndef TABLES_H
#define TABLES_H

#include <stdint.h>

namespace x86_tables {

#pragma pack(push, 0)
struct idt_gate {
    uint16_t offset_1;
    uint16_t selector;
    uint8_t  ist = 0;
    uint8_t  type_attr;
    uint16_t offset_2;
    uint32_t offset_3;
    uint32_t zero = 0;
} __attribute__((packed));

struct gdt_segment {
    uint16_t limit_1;
    uint16_t base_1;
    uint8_t  base_2;
    uint8_t  access;
    uint8_t  flags_limit_2;
    uint8_t  base_3;
    uint32_t base_4;
    uint32_t zero = 0;
} __attribute__((packed));
#pragma pack(pop)

enum gdt_selector : uint16_t {
    null_selector = 0,
    code_selector = 0x08,
    data_selector = 0x10,
};

void* get_offset(idt_gate target);

void set_gate(idt_gate& target, void* offset, gdt_selector selector,
              uint8_t min_privilege, uint8_t type, bool present);

enum gdt_access_types : uint8_t {
    null_access = 0,
    code_access = 0x9a, // 0b 1001 1010
    data_access = 0x92, // 0b 1001 0010
    tss_access  = 0x89
};

enum gdt_flags : uint8_t {
    long_paged = 0b10100000,
    long_bytes = 0b00100000,
    p_32_paged = 0b11000000,
    p_32_bytes = 0b01000000,
    p_16_paged = 0b10000000,
    p_16_bytes = 0b00000000,
    null_flags = 0,
};

void set_segment(gdt_segment& target, void* base, uint32_t limit,
                 gdt_access_types access, gdt_flags flags = long_paged);
} // namespace x86_tables

#endif
#ifndef TABLES_H
#define TABLES_H

#include "libk/asm.h"

#include <stdint.h>

namespace x86_tables {

#pragma pack(push, 0)
struct idt_gate {
    uint16_t offset_1;
    uint16_t selector;
    uint8_t  ist = 1;
    uint8_t  type_attr;
    uint16_t offset_2;
    uint32_t offset_3;
    uint32_t zero = 0;
} __attribute__((packed));

void* get_offset(idt_gate target);

struct gdt_segment {
    uint16_t limit_1;
    uint16_t base_1;
    uint8_t  base_2;
    uint8_t  access;
    uint8_t  flags_limit_2;
    uint8_t  base_3;

    enum segment_type : uint8_t {
        null_segment = 0,
        code         = 0x9a, // 0b 1001 1010
        data         = 0x92, // 0b 1001 0010

        ldt = 0x82,
        tss = 0x89,
        ist = 0x89
    };

    enum privilege_type : uint8_t { kernel = 0, user = 3 << 5 };

    enum flags_type : uint8_t {
        p_64_code  = 0b00100000,
        p_64_data  = 0,
        p_32_paged = 0b11000000,
        p_32_bytes = 0b01000000,
        p_16_paged = 0b10000000,
        p_16_bytes = 0b00000000,
        null_flags = 0,
    };

    gdt_segment(segment_type type, privilege_type privilege_level,
                bool code = false) {
        access        = (uint8_t)type | (uint8_t)privilege_level;
        flags_limit_2 = (code ? p_64_code : null_flags);
    }

    void set(segment_type type, privilege_type privilege_level,
             bool code = false) {
        access        = (uint8_t)type | (uint8_t)privilege_level;
        flags_limit_2 = (code ? p_64_code : null_flags);
    }

} __attribute__((packed));

// An expanded segment descriptor is used for LDT and TSS/IST segments
struct gdt_system_segment : public gdt_segment {
    uint32_t base_4;
    uint32_t zero = 0;

    gdt_system_segment(segment_type type, privilege_type privilege_level,
                       uint64_t base, uint32_t limit, bool code = false)
        : gdt_segment(type, privilege_level, code) {
        base_1 = base & (0xffff);
        base >>= 16;
        base_2 = base & 0xff;
        base >>= 8;
        base_3 = base & 0xff;
        base >>= 8;
        base_4 = base & 0xffffffff;

        limit_1 = limit & 0xffff;
        limit >>= 16;
        flags_limit_2 = (flags_limit_2 & 0xf0) | (limit & 0xf);
    }

    void set_special(uint64_t base, uint32_t limit) {
        base_1 = base & (0xffff);
        base >>= 16;
        base_2 = base & 0xff;
        base >>= 8;
        base_3 = base & 0xff;
        base >>= 8;
        base_4 = base & 0xffffffff;

        limit_1 = limit & 0xffff;
        limit >>= 16;
        flags_limit_2 = (flags_limit_2 & 0xf0) | (limit & 0xf);
    }
};

// GDT follows set format for x86_64
struct gdt_table {
    gdt_segment null_segment;

    gdt_segment kernel_code;
    gdt_segment kernel_data;

    gdt_segment user_code;
    gdt_segment user_data;

    gdt_system_segment ist_segment;

    gdt_table()
        : null_segment(gdt_segment::null_segment, gdt_segment::kernel, false)
        , kernel_code(gdt_segment::code, gdt_segment::kernel, true)
        , kernel_data(gdt_segment::data, gdt_segment::kernel, false)
        , user_code(gdt_segment::code, gdt_segment::user, true)
        , user_data(gdt_segment::data, gdt_segment::user, false)
        , ist_segment(gdt_segment::ist, gdt_segment::kernel, 0, 0, false) {}

    void set_ist(uint64_t base, uint16_t limit) {
        ist_segment.set_special(base, limit);
    }

    void load_gdt() { set_gdt(this, sizeof(gdt_table)); }
};

// Selectors index into gdt entries
// bit
// 0-1 privilege level
// 2   table (0 = gdt, 1 = ldt)
// 3-15 entry index
enum gdt_selector : uint16_t {
    null_selector = 0,

    code_selector = 0x08,
    data_selector = 0x10,

    user_code_selector = 0x1b,
    user_data_selector = 0x23,

    ist_selector = 0x28
};

// x86_64 version of the TSS
// Stores stack pointers to switch to for interrupts and
//  privilege level changes
struct interrupt_stack_table {
    uint32_t _reserved;
    uint64_t privilege_level_stack[3];
    uint64_t interrupt_stack[8];
    uint64_t __reserved;
    uint16_t ___reserved;
    uint16_t iopb_offset = 104;

    void load_ist() { set_tss(ist_selector); }
} __attribute__((packed));
#pragma pack(pop)

void set_gate(idt_gate& target, void* offset, gdt_selector selector,
              uint8_t min_privilege, uint8_t type, bool present);

} // namespace x86_tables

#endif

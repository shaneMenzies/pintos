#ifndef IO_H
#define IO_H

#include <stdint.h>

#include "asm_functions.h"
#include "acpi.h"

#define HPET_PERIOD_CONSTANT 1000000000000000UL

enum IO_ports : uint16_t{
    PIC_1 = 0x20,
    PIC_1_CMD = (PIC_1),
    PIC_1_DATA = (PIC_1 + 1),

    PIC_2 = 0xa0,
    PIC_2_CMD = (PIC_2),
    PIC_2_DATA = (PIC_2 + 1),

    KB_DATA = 0x60,
    KB_CMD = 0x64,

    PIT_CH0 = 0x40,
    PIT_CH1 = 0x41,
    PIT_CH2 = 0x42,
    PIT_CMD = 0x43,

    COM_1 = 0x3f8,
    COM_2 = 0x2f8,
};

enum PIC_commands : uint8_t {
    OFFSET_1 = 0x20,
    OFFSET_2 = (OFFSET_1 + 8),
    OFFSET_END = (OFFSET_2 + 8),

    CODE_INIT = 0x11,
    CODE_HAS_SLAVE = 0x4,
    CODE_IS_SLAVE = 0x2,

    CODE_8086 = 0x1,

    CODE_EOI = 0x20,
};

union io_apic_redirection_entry {
    uint64_t raw_data;
    struct {
        uint64_t int_vector : 8;
        uint64_t delivery_mode : 3;
        uint64_t dest_mode : 1;
        uint64_t apic_busy : 1;
        uint64_t polarity : 1;
        uint64_t int_recieved : 1;
        uint64_t trigger_mode : 1;
        uint64_t masked : 1;
        uint64_t reserved : 39;
        uint64_t destination : 8;
    } data __attribute__ ((packed));

    io_apic_redirection_entry(uint64_t value) : raw_data(value) {}
};

struct io_apic {
    uint32_t reg_select;
    uint32_t unused[3];
    uint32_t reg_data;

    void write_32(uint32_t index, uint32_t data) volatile {
        reg_select = index;
        reg_data = data;
    };
    uint32_t read_32(uint32_t index) volatile {
        reg_select = index;
        return reg_data;
    };

    void write_64(uint32_t index, uint64_t data) volatile {
        reg_select = index;
        reg_data = (0U | (data & 0xffffffff));
        reg_select = (index + 1);
        reg_data = (0U | (data >> 32));
    };
    uint64_t read_64(uint32_t index) volatile {
        reg_select = index;
        uint64_t value = reg_data;
        reg_select = (index + 1);
        value |= ((reg_data | 0ULL) << 32);
        return value;
    };

    void set_interrupt(uint8_t apic_int, io_apic_redirection_entry value) {
        write_64(((apic_int * 2) + 0x10), value.raw_data);
    };
    io_apic_redirection_entry get_interrupt(uint8_t apic_int) {
        return (io_apic_redirection_entry)(read_64((apic_int * 2) + 0x10));
    };

    void set_vector(uint8_t apic_int, uint8_t new_vector) {
        write_32(((apic_int * 2) + 0x10), 
            ((read_32((apic_int * 2) + 0x10) & ~(0xff)) | new_vector));
    };
    uint8_t get_vector(uint8_t apic_int) {
        return (read_32((apic_int * 2) + 0x10) & 0xff);
    };

    void set_masked(uint8_t apic_int, bool value) {
        write_32(((apic_int * 2) + 0x10), 
            ((read_32((apic_int * 2) + 0x10) & ~(1 << 16)) | (value ? (1 << 16) : 0)));
    };

} __attribute__ ((packed));

struct io_apic_info {
    io_apic* address;
    uint8_t num_ints;
    uint8_t start;
    uint8_t end;
    uint8_t id;

    void fill(acpi::madt_io_apic* source);
};

struct hpet_comparator {
    volatile uint64_t* address;
    unsigned int index;
    uint32_t valid_irqs;
    uint8_t current_irq;
    bool periodic_capable;
    bool used;
};

struct hpet_info {
    acpi::hpet_table* acpi_table;
    volatile uint64_t* address;
    uint64_t rate;
    uint64_t min_tick;
    bool legacy_capable;
    bool long_capable;
    unsigned int num_comparators;
    hpet_comparator* comparators;

    volatile uint64_t* get_comparator_address(int index) const {
        return (volatile uint64_t*)((uintptr_t)address + 0x100 + (0x20 * index));
    };

    hpet_info(acpi::hpet_table* table);
};

namespace serial {

    void write_c(const char data, uint16_t port);
    void write_s(const char* string, uint16_t port);
}

#endif
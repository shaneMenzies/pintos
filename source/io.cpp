/**
 * @file io.cpp
 * @author Shane Menzies
 * @brief I/O Functionanality
 * @date 04/21/21
 * 
 * 
 */

#include "io.h"
#include "paging.h"

void io_apic_info::fill(acpi::madt_io_apic* source) {
    address = (io_apic*)(source->io_apic_addr | 0ULL);
    id = source->io_apic_id;
    start = (source->global_int_base & 0xff);
    num_ints = ((address->read_32(1) >> 16) & 0xff);
    end = start + num_ints;
}

hpet_info::hpet_info(acpi::hpet_table* table) : acpi_table(table) {
    address = (volatile uint64_t*)(table->address.address);
    min_tick = table->minimum_tick;
    legacy_capable = (table->legacy_replacement ? true : false);
    paging::identity_map_region((uintptr_t)address, 0x20);
    if (legacy_capable) {
        // Disable legacy replacement
        address[1] &= ~(2);
    }
    num_comparators = ((address[0] >> 8) & 0x1f) + 1;
    paging::identity_map_region((uintptr_t)address, (0x100 + (0x20 * num_comparators)));
    comparators = new hpet_comparator[num_comparators];
    long_capable = (table->counter_size ? true : false);

    // Convert from the tick period in femtoseconds(10^-15) to a rate in Hz
    rate = (uint64_t)((HPET_PERIOD_CONSTANT) / ((*address) >> 32));

    // Fill info on the comparators
    volatile uint64_t* next_comparator = (volatile uint64_t*)((uintptr_t)address + 0x100);
    for (unsigned int i = 0; i < num_comparators; i++) {

        // Fill out info on this comparator
        comparators[i].address = next_comparator;
        comparators[i].index = i;
        comparators[i].used = false;
        comparators[i].periodic_capable = (next_comparator[0] & (1<<4)) ? true : false;
        comparators[i].valid_irqs = (next_comparator[0] >> 32) & 0xffffffff;
        comparators[i].current_irq = (next_comparator[0] >> 9) & 0xb11111;

        // Set all values to known defaults
        next_comparator[0] &= ~((1<<14) | (1<<8) | (1<<3) | (1<<2) | (1<<1));

        next_comparator = (volatile uint64_t*)((uintptr_t)address + 0x20);
    }
}

namespace serial {

    void write_c(const char data, uint16_t port) {

        out_byte(data, port);
    }

    void write_s(const char* string, uint16_t port) {

        int i = 0;
        while (1) {

            // Get next char
            char data = string[i];

            // Check for null termination
            if (data == '\0') {
                break;
            } else {
                out_byte(data, port);
                i++;
            }
        }
    }

}

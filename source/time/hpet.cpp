/**
 * @file hpet_comparator.cpp
 * @author Shane Menzies
 * @brief
 * @date 7/3/22
 *
 *
 */

#include "hpet.h"

#include "memory/addressing.h"
#include "memory/paging.h"

#define HPET_PERIOD_CONSTANT 1000000000000000UL

using timestamp = timable_device<>::timestamp;

hpet::hpet(acpi::hpet_table* table)
    : device(default_name, default_model, nullptr, 0)
    , acpi_table(table) {
    address        = (volatile uint64_t*)(table->address.address);
    min_tick       = table->minimum_tick;
    legacy_capable = (table->legacy_replacement ? true : false);
    paging::kernel_address_space.identity_map_region((uintptr_t)address, 0x20);
    if (legacy_capable) {
        // Disable legacy replacement
        address[1] = address[1] & ~(2);
    }

    // Stop HPET counter for now
    address[2] = address[2] & ~(1);

    // Map region for comparators
    num_comparators = ((address[0] >> 8) & 0x1f) + 1;
    paging::kernel_address_space.identity_map_region(
        (uintptr_t)address, (0x100 + (0x20 * num_comparators)));
    comparators = (hpet_comparator<true, false>*)malloc(
        sizeof(hpet_comparator<true, false>) * num_comparators);
    long_capable = (table->counter_size ? true : false);

    // Convert from the tick period in femtoseconds(10^-15) to a rate in Hz
    rate = (uint64_t)((HPET_PERIOD_CONSTANT) / ((*address) >> 32));

    // Register HPET before the comparators
    devices::register_device(this, default_path, &devices::device_tree);

    // Fill info on the comparators
    volatile uint64_t* next_comparator
        = (volatile uint64_t*)((uintptr_t)address + 0x100);
    for (unsigned int i = 0; i < num_comparators; i++) {

        // Fill out info on this comparator
        new (&comparators[i]) hpet_comparator<true, false>(this, i);

        next_comparator
            = (volatile uint64_t*)((uintptr_t)next_comparator + 0x20);
    }

    // Restart counter
    main_counter() = 0;
    address[2]     = (address[2] & ~(0b11)) | 1;
}

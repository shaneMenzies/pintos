/**
 * @file io.cpp
 * @author Shane Menzies
 * @brief I/O Functionanality
 * @date 04/21/21
 * 
 * 
 */

#include "io.h"

void io_apic_info::fill(acpi::madt_io_apic* source) {
    address = (io_apic*)(source->io_apic_addr | 0ULL);
    id = source->io_apic_id;
    start = (source->global_int_base & 0xff);
    num_ints = ((address->read_32(1) >> 16) & 0xff);
    end = start + num_ints;
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

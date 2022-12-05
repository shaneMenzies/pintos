/**
 * @file io.cpp
 * @author Shane Menzies
 * @brief I/O Functionanality
 * @date 04/21/21
 *
 *
 */

#include "io.h"

#include "libk/asm.h"
#include "memory/addressing.h"
#include "memory/paging.h"
#include "system/acpi.h"

char io_read_c(uint16_t port) { return in_byte(port); }

void io_write_c(const char data, uint16_t port) { out_byte(data, port); }

void io_write_s(const char* string, uint16_t port) {

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

namespace serial {
serial_device serial_handlers[NUM_SERIAL_HANDLERS];
}

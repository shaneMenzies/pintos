/**
 * @file gdb_compat.cpp
 * @author Shane Menzies
 * @brief Functions for compatibility with gdb stub
 * @date 04/10/22
 *
 *
 */

#include "debug_build.h"
#include "io/io.h"
#include "stdint.h"

const uint16_t debug_port = IO_ports::COM_2;
#define DEBUG_INITIAL_BUFFER_SIZE 4096
char              debug_initial_buffer[DEBUG_INITIAL_BUFFER_SIZE];
io_buffer_device* debug_serial_buffer;

void prepare_debug_serial() {
    debug_serial_buffer = new io_buffer_device(debug_port, debug_initial_buffer,
                                               DEBUG_INITIAL_BUFFER_SIZE);

    serial::serial_handlers[serial::serial_handler_identities::COM_2].handler
        = debug_serial_buffer;

    serial::serial_handlers[serial::serial_handler_identities::COM_2]
        .initialize_port(300, true);
}

/*
extern "C" {
    void* memset(void* ptr, int value, size_t num) {
        char* target = (char*)ptr;
        char data = (value & 0xff);

        for (size_t i = 0; i < num; i++) {
            target[i] = data;
        }

        return ptr;
    }

    void exceptionHandler(int exception_number, void* exception_address) {
        interrupts::set_direct_interrupt((exception_number & 0xff),
interrupts::gate_type::INT_GATE_32, (void
(*)(interrupt_frame*))exception_address);
    }

    int getDebugChar() {
        return (int)debug_serial_buffer.read();
    }

    void putDebugChar(int data) {
        io_write_c(data & 0xff, debug_port);
    }

    int sprintf(const char* format, ...) {
        va_list args;
        va_start(args, format);
        active_terminal->vtprintf(format, args);
        va_end(args);
    }

    char* strcpy(char* dest, const char* source) {
        strcpy(dest, source);
        return dest;
    }
}
*/

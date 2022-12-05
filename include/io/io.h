#ifndef IO_H
#define IO_H

#include "libk/asm.h"
#include "libk/queue.h"

#include <stdint.h>

namespace acpi {
struct madt_io_apic;
struct hpet_table;
} // namespace acpi

enum IO_ports : uint16_t {
    PIC_1      = 0x20,
    PIC_1_CMD  = (PIC_1),
    PIC_1_DATA = (PIC_1 + 1),

    PIC_2      = 0xa0,
    PIC_2_CMD  = (PIC_2),
    PIC_2_DATA = (PIC_2 + 1),

    KB_DATA = 0x60,
    KB_CMD  = 0x64,

    PIT_CH0 = 0x40,
    PIT_CH1 = 0x41,
    PIT_CH2 = 0x42,
    PIT_CMD = 0x43,

    COM_1 = 0x3f8,
    COM_2 = 0x2f8,
};

enum PIC_commands : uint8_t {
    OFFSET_1   = 0x20,
    OFFSET_2   = (OFFSET_1 + 8),
    OFFSET_END = (OFFSET_2 + 8),

    CODE_INIT      = 0x11,
    CODE_HAS_SLAVE = 0x4,
    CODE_IS_SLAVE  = 0x2,

    CODE_8086 = 0x1,

    CODE_EOI = 0x20,
};

char io_read_c(uint16_t port);

void io_write_c(const char data, uint16_t port);
void io_write_s(const char* string, uint16_t port);

class io_bus_device {

  public:
    io_bus_device() {}

    void write_to_port(const char value) { io_write_c(value, associated_port); }

    char read_from_port() { return io_read_c(associated_port); }

    uint16_t get_port() { return associated_port; }

    virtual void write(const char value) { write_to_port(value); }

    virtual char read() { return read_from_port(); }

    virtual void device_interrupt(char data) = 0;

  protected:
    uint16_t associated_port;
};

#define DEFAULT_BUFFER_SIZE 2048
class io_buffer_device : public io_bus_device {

  public:
    io_buffer_device(uint16_t port)
        : buffer(DEFAULT_BUFFER_SIZE) {
        associated_port = port;
    }
    io_buffer_device(uint16_t port, char* initial_buffer,
                     size_t initial_buffer_size)
        : buffer(initial_buffer, initial_buffer_size) {
        associated_port = port;
    }

    char read() override {
        char value = buffer.front();
        buffer.pop();
        return value;
    }

    void write_to_buffer(const char value) { buffer.push(value); }

    void clear() { buffer.clear(); }

    size_t current_size() { return buffer.size(); }

    void device_interrupt(char data) override { write_to_buffer(data); }

  protected:
    std_k::queue<char> buffer;
};

namespace serial {

#define UART_INTERNAL_RATE 115200
struct serial_device {

    serial_device()
        : handler(nullptr) {}
    serial_device(io_bus_device* handler)
        : handler(handler) {}

    char read() {
        if (handler != nullptr)
            return handler->read();
        else
            return 0;
    }

    void write(const char value) {
        if (handler != nullptr) handler->write(value);
    }

    void device_interrupt() {
        if (handler != nullptr) {
            char value = handler->read_from_port();
#ifdef DEBUG
            // Check for ^C ('\003'), to trigger breakpoint
            if (value == '\003') { trigger_breakpoint(); }
#endif

            handler->device_interrupt(value);
        }
    }

    void initialize_port(uint32_t desired_rate = 300,
                         bool interrupt_driven = false, uint8_t data_bits = 8,
                         uint8_t parity_bits = 0, uint8_t stop_bits = 1) {
        uint16_t base = handler->get_port();

        // Set appropriate divider
        uint16_t divisor = (UART_INTERNAL_RATE / desired_rate);
        io_write_c(0x80 | io_read_c(base + 3), base + 3);    // Set DLAB
        io_write_c((divisor & 0xff), base);                  // Low byte
        io_write_c(((divisor & 0xff00) >> 8), base + 1);     // High byte
        io_write_c(io_read_c(base + 3) & ~(0x80), base + 3); // Unset DLAB

        // Bits config
        uint8_t bit_format = ((parity_bits & 0b111) << 3)
                             | ((stop_bits == 1) ? 0b00 : 0b100)
                             | ((data_bits - 5) & 0b11);
        bit_format |= (io_read_c(base + 3) & 0xc0);
        io_write_c(bit_format, base + 3);

        // Interrupt-enable
        io_write_c((interrupt_driven ? 1 : 0), base + 1);
    }

    uint8_t get_line_status() { return io_read_c(handler->get_port() + 5); }

    bool data_waiting() { return (get_line_status() & 0b1); }

    bool ready_to_send() { return (get_line_status() & 0x20); }

    io_bus_device* handler;
};

#define NUM_SERIAL_HANDLERS 2
enum serial_handler_identities { COM_1, COM_2 };
extern serial_device serial_handlers[NUM_SERIAL_HANDLERS];
} // namespace serial

#endif

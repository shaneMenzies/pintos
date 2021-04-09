#ifndef IO_H
#define IO_H

#include <stdint.h>

enum IO_ports : uint8_t{
    PIC_1 = 0x20,
    PIC_1_CMD = (PIC_1),
    PIC_1_DATA = (PIC_1 + 1),

    PIC_2 = 0xa0,
    PIC_2_CMD = (PIC_2),
    PIC_2_DATA = (PIC_2 + 1),

    OFFSET_1 = 0x20,
    OFFSET_2 = (OFFSET_1 + 8),
    OFFSET_END = (OFFSET_2 + 8),

    CODE_INIT = 0x11,
    CODE_HAS_SLAVE = 0x4,
    CODE_IS_SLAVE = 0x2,

    CODE_8086 = 0x1,

    CODE_EOI = 0x20,

    KB_DATA = 0x60,
    KB_CMD = 0x64,

    INT_GATE_32 = 0x0E,
    INT_GATE_16 = 0x06,

    TRAP_GATE_32 = 0xf,
    TRAP_GATE_16 = 0x7,

    TASK_GATE = 0x05,

    PIT_CH0 = 0x40,
    PIT_CH1 = 0x41,
    PIT_CH2 = 0x42,
    PIT_CMD = 0x43,
};

extern "C" {
    extern void out_byte(unsigned char byte, uint16_t port);
    extern unsigned char in_byte(uint16_t port);

    extern void out_word(uint16_t word, uint16_t port);
    extern uint16_t in_word(uint16_t port);

    extern void out_dword(uint32_t dword, uint16_t port);
    extern uint32_t in_dword(uint16_t port);

    extern void io_wait();
}

#endif
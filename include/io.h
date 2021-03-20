#ifndef IO_H
#define IO_H

#include <stdint.h>

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
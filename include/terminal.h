#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdarg.h>
#include <stdint.h>

#include "memory.h"
#include "display.h"
#include "libk.h"

typedef class terminal {
    private:
        char* start;
        char* next;
        char* end;

        char keyboard[1024];
        unsigned char kb_flags = 0;

    public:
        uint32_t default_fg, default_bg;
        uint8_t default_ega;

        terminal(size_t text_size = 65536, uint32_t fg = 0xffffffu, uint32_t bg = 0u, uint8_t ega = 0x0f);
        ~terminal();

        void write(const char*);
        void printf(const char*, ...);

        void show();
        void show(uint32_t fg, uint32_t bg, uint8_t ega);
} terminal;

uint16_t stringify(char* target_buffer, int number, uint8_t base);

#endif
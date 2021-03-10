#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

typedef class terminal {
    private:
        char* start;
        char* next;
        char* end;

        char keyboard[1024];

    public:
        uint32_t default_fg, default_bg;
        uint8_t default_ega;

        terminal(size_t text_size = 65536, uint32_t fg = 0xffffffu, uint32_t bg = 0u, uint8_t ega = 0x0f);
        ~terminal();

        void write(char*);
        void printf(char*, ...);

        void show();
        void show(uint32_t fg, uint32_t bg, uint8_t ega);
} terminal;

uint16_t stringify(char* target_buffer, int number, uint8_t base);

#endif
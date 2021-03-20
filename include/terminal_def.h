#ifndef TERMINAL_DEF_H
#define TERMINAL_DEF_H

#include <stdint.h>
#include <stddef.h>

enum {
    KB_BUF_SIZE = 1024,
};

typedef class terminal {
    private:
        char* start;
        char* next;
        char* end;

        char keyboard[KB_BUF_SIZE];
        uint16_t kb_index;
        unsigned char kb_flags = 0;

    public:
        uint32_t default_fg, default_bg;
        uint8_t default_ega;

        terminal(size_t text_size = 65536, uint32_t fg = 0xffffff, uint32_t bg = 0, uint8_t ega = 0x0f);
        ~terminal();

        void kb_append_c(char new_char);
        void kb_append_s(char* string);
        void kb_clear();

        void write(const char*);
        void printf(const char*, ...);

        void clear();

        void show();
        void show(uint32_t fg, uint32_t bg, uint8_t ega);
} terminal;

#endif
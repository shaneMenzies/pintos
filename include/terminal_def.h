#ifndef TERMINAL_DEF_H
#define TERMINAL_DEF_H

#include <stdint.h>
#include <stddef.h>
#include "display.h"

enum {
    KB_BUF_SIZE = 1024,
};

class terminal {

    friend class visual_terminal;

    private:
        char* start;
        char* next;
        char* end;

        char keyboard[KB_BUF_SIZE];
        uint16_t kb_index;
        unsigned char kb_flags = 0;

    public:
        terminal(size_t text_size = 65536);
        ~terminal();

        void kb_append_c(char new_char);
        void kb_append_s(const char* string);
        void kb_clear();

        void write(const char* string);
        void tprintf(const char* format, ...);

        void clear();
};

class visual_terminal : public terminal {

    private:
        class v_fb fb;
        uint32_t x_pos = 0;
        uint32_t y_pos = 0;

    public:
        uint32_t default_fg, default_bg;
        uint8_t default_ega;
        unsigned int target_lines;

        visual_terminal(size_t text_size = 65536, uint32_t fg = 0xffffff, 
                        uint32_t bg = 0, uint8_t ega = 0x0f, unsigned int target_lines = 30);

        void write(const char* string);
        void tprintf(const char* format, ...);

        void clear();

        void show();
        void show(uint32_t fg, uint32_t bg, uint8_t ega);
};

#endif
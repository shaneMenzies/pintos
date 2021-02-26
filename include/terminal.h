#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

typedef class terminal {
    private:
        char text[4147200];
        unsigned int index;
        unsigned int max_chars;

    public:
        uint32_t default_fg, default_bg;
        uint8_t default_ega;

        terminal(uint32_t, uint32_t, uint8_t);
        ~terminal();

        void write(char*);
        void printf(char*, ...);

        void show();
        void show(uint32_t, uint32_t);
        void show(uint8_t);
} terminal;

uint16_t stringify(char* target_buffer, int number, uint8_t base);

void tprintf(terminal* target_term, char* format, ...);

#endif
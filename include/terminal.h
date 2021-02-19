#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

typedef struct terminal {
    uint32_t default_fg;
    uint32_t default_bg;
    uint8_t default_ega;

    unsigned int max_chars;
    char text[4147200];

    unsigned int index;
} terminal;

terminal create_terminal(uint32_t fg_color, uint32_t bg_color, 
                         uint8_t ega_attributes, unsigned int max_chars);

void terminal_write(terminal* target_terminal, char string[]);

void terminal_print(terminal* target_terminal);

#endif
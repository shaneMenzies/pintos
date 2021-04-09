#ifndef ERROR_H
#define ERROR_H

#include "terminal_def.h"

#include <stdint.h>

struct error_code_section {
    uint32_t code;
    char* caller;
};

extern struct error_code_section* error_code_addr;

void set_error_terminal(visual_terminal* new_terminal);

void check_error_terminal();

void raise_error(uint32_t error_code, char* caller);

const char* get_code_info(uint32_t error_code);

#include "terminal.h"

#endif
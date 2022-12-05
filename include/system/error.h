#ifndef ERROR_H
#define ERROR_H

#include <stdint.h>

struct visual_terminal;

void set_error_terminal(visual_terminal* new_terminal);

void check_error_terminal();

void raise_error(uint32_t error_code, const char* caller);

const char* get_code_info(uint32_t error_code);

#endif
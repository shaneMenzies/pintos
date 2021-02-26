#ifndef ERROR_H
#define ERROR_H

#include "terminal.h"

#include <stdint.h>

extern uint32_t* error_code_addr;
extern char** error_caller_addr;

void set_error_terminal(terminal* new_terminal);

void check_error_terminal();

void raise_error(uint32_t error_code, char* caller);

const char* get_code_info(uint32_t error_code);

#endif
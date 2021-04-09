#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdarg.h>
#include <stdint.h>

#include "terminal_def.h"
#include "memory.h"
#include "display.h"
#include "libk.h"

extern visual_terminal* boot_terminal;
extern visual_terminal* active_terminal;

void draw_active_cursor();

uint16_t stringify(char* target_buffer, int number, uint8_t base);

#endif
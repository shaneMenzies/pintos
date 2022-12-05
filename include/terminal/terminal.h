#ifndef TERMINAL_H
#define TERMINAL_H

#include "terminal/terminal_def.h"

#include <stdarg.h>
#include <stdint.h>

struct visual_terminal;

extern visual_terminal* boot_terminal;
extern visual_terminal* active_terminal;

void draw_active_cursor();

#endif
#ifndef GDT_H
#define GDT_H

#include "interrupts.h"
#include "x86_tables.h"
#include "p_memory.h"

void gdt_init();

#endif
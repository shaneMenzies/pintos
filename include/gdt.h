#ifndef GDT_H
#define GDT_H

#include "interrupts.h"
#include "x86_tables.h"
#include "memory.h"

void gdt_init();

#endif
#ifndef INIT_H
#define INIT_H

#include "kernel.h"
#include "interrupts.h"
#include "error.h"
#include "x86_tables.h"
#include "multiboot.h"
#include "display.h"
#include "gdt.h"

extern "C" {
uint32_t return_ebx();
}

void __cxa_pure_virtual();

void kernel_init();

#endif
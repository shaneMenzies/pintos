#ifndef INIT_H
#define INIT_H

#include "kernel.h"
#include "interrupts.h"
#include "error.h"
#include "x86_tables.h"
#include "multiboot.h"
#include "display.h"
#include "gdt.h"
#include "timer.h"

extern "C" {
uint32_t return_ebx();
bool fpu_init();

extern uint16_t fpu_status;
}

extern bool float_support;

void __cxa_pure_virtual();

#endif
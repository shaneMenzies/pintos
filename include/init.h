#ifndef INIT_H
#define INIT_H

#include "multiboot.h"
#include "io.h"
#include "asm_functions.h"
#include "kernel.h"
#include "interrupts.h"
#include "error.h"
#include "x86_tables.h"
#include "display.h"
#include "timer.h"
#include "commands.h"
#include "paging.h"
#include "pintos_std.h"
#include "threading.h"

extern "C" {

void _start(multiboot_boot_info* mb_info);
void early_init();
extern void _init();
void late_init(multiboot_boot_info* mb_info);
extern void _fini();

extern bool float_support;
}


void __cxa_pure_virtual();

#endif
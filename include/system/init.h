#ifndef INIT_H
#define INIT_H

struct multiboot_boot_info;

extern "C" {

void        _start(multiboot_boot_info* mb_info);
void        early_init(multiboot_boot_info* mb_info);
extern void _init();
void        late_init(multiboot_boot_info* mb_info);
extern void _fini();

extern bool float_support;
extern bool initialized;
}

void __cxa_pure_virtual();

#endif

#ifndef STACK_CHECK_H
#define STACK_CHECK_H

#include <stdint.h>

extern "C" {
extern uintptr_t __stack_chk_guard;

__attribute__((noreturn)) void __stack_chk_fail();
}

#endif
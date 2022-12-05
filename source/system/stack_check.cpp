/**
 * @file stack_check.cpp
 * @author Shane Menzies
 * @brief Support for GCC Stack Checking
 * @date 02/21/22
 *
 *
 */

#include "error.h"
#include "libk/asm.h"
#include "libk/random.h"
#include "pintos_std.h"

#include <stdint.h>

extern "C" {
uintptr_t __stack_chk_guard = std_k::get_rand();

__attribute__((noreturn)) void __stack_chk_fail() {
    raise_error(666, "User Process");
    disable_interrupts();
    while (1) {}
}
}

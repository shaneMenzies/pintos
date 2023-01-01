#ifndef PINTOS_SYSCALL_H
#define PINTOS_SYSCALL_H

#include "libk/common.h"

// System calls are called with 6 args, even if they don't use them all
#define SYSCALL_ARG_TYPES \
    uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t
#define SYSCALL_ARGS                                                \
    uint64_t arg_0, uint64_t arg_1, uint64_t arg_2, uint64_t arg_3, \
        uint64_t arg_4, uint64_t arg_5
#define SYSCALL_ARG_NAMES arg_0, arg_1, arg_2, arg_3, arg_4, arg_5

__attribute__((regparm(1))) uint64_t      syscall(uint64_t id, SYSCALL_ARGS);
__attribute__((indirect_return)) uint64_t syscall_handler();

#endif // PINTOS_SYSCALL_H

#ifndef PINTOS_SYSCALL_INDEX_H
#define PINTOS_SYSCALL_INDEX_H

#include "libk/functional.h"
#include "syscall.h"

extern std_k::function<uint64_t(SYSCALL_ARG_TYPES)> syscall_index[];

namespace syscalls {
enum id : uint64_t {
    read  = 0,
    write = 1,
    open  = 2,
    close = 3,
};
}

#endif // PINTOS_SYSCALL_INDEX_H

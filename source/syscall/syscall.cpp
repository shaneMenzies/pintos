/**
 * @file sycall.cpp
 * @author Shane Menzies
 * @brief
 * @date 12/31/22
 *
 *
 */

#include "syscall.h"

#include "libk/asm.h"
#include "syscall_index.h"
#include "threading/topology.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"

__attribute__((regparm(1))) uint64_t syscall(uint64_t id, SYSCALL_ARGS) {
    // Arguments need to be passed in a special order to avoid clobbered
    // registers ID will actually be passed in rax syscall uses rcx and r11 to
    // store rip and rflags
    asm volatile("movq %0, %%rdi \n\t"
                 "movq %1, %%rsi \n\t"
                 "movq %2, %%rdx \n\t"
                 "movq %3, %%r10 \n\t"
                 "movq %4, %%r8 \n\t"
                 "movq %5, %%r9 \n\t"
                 "movq %[id], %%rax \n\t"
                 "syscall \n\t" ::"g"(arg_0),
                 "g"(arg_1), "g"(arg_2), "g"(arg_3), "g"(arg_4),
                 "g"(arg_5), [id] "g"(id)
                 : "rax", "rcx", "rdx", "rdi", "rsi", "r8", "r9", "r10", "r11");
}

__attribute__((indirect_return)) uint64_t syscall_handler() {
    // Need to save return info passed in rcx and r11
    uint64_t id;
    uint64_t saved_rcx;
    uint64_t saved_r11;
    asm volatile("movq %%rcx, %[saved_rcx] \n\t"
                 "movq %%r11, %[saved_r11] \n\t"
                 : [id] "=r"(id), [saved_rcx] "=g"(saved_rcx),
                   [saved_r11] "=g"(saved_r11));

    // Swap to system stack
    uint64_t system_stack = (uint64_t)current_thread()->system_stack;
    uint64_t saved_rsp;
    uint64_t saved_rbp;
    asm volatile("movq %%rsp, %[saved_rsp] \n\t"
                 "movq %%rbp, %[saved_rbp] \n\t"
                 "movq %[new_stack], %%rsp \n\t"
                 "movq %%rsp, %%rbp \n\t"
                 "pushq %[saved_rsp] \n\t"
                 "pushq %[saved_rbp] \n\t"
                 : [saved_rsp] "=r"(saved_rsp), [saved_rbp] "=r"(saved_rbp)
                 : [new_stack] "r"(system_stack));

    // Need to move arguments into place
    uint64_t SYSCALL_ARG_NAMES;
    asm volatile("movq %%rdi,%0  \n\t"
                 "movq %%rsi,%1  \n\t"
                 "movq %%rdx,%2  \n\t"
                 "movq %%r10,%3  \n\t"
                 "movq %%r8 ,%4 \n\t"
                 "movq %%r9 ,%5 \n\t"
                 : "=g"(arg_0), "=g"(arg_1), "=g"(arg_2), "=g"(arg_3),
                   "=g"(arg_4), "=g"(arg_5));

    // Can call target syscall
    id = syscall_index[id](SYSCALL_ARG_NAMES);

    // Swap back to caller stack
    asm volatile("popq %rbp \n\t"
                 "popq %rsp \n\t");

    // Restore return info and return
    asm volatile("movq %[result], %%rax \n\t"
                 "movq %[saved_rcx], %%rcx \n\t"
                 "movq %[saved_r11], %%r11 \n\t"
                 "sysretq \n\t" ::[saved_rcx] "g"(saved_rcx),
                 [saved_r11] "g"(saved_r11), [result] "g"(id));
}

#pragma GCC diagnostic pop

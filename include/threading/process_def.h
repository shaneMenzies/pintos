#ifndef PINTOS_PROCESS_DEF_H
#define PINTOS_PROCESS_DEF_H

#include "interrupts/interrupts.h"
#include "libk/asm.h"
#include "libk/callable.h"
#include "libk/functional.h"
#include "libk/ostream.h"
#include "libk/vector.h"
#include "memory/addressing.h"
#include "memory/chunking_predef.h"
#include "memory/common_region.h"
#include "memory/p_memory.h"
#include "terminal/terminal.h"
#include "threading/topology.h"
#include "time/timer.h"

#include <cpuid.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define DEFAULT_STACK_SIZE 8192

class allocation_manager;

namespace threading {
struct processor_state {
    uint64_t rip;
    uint64_t rflags = 1 << 9;
    uint64_t rsp;
    uint64_t rbp;

    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;

    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;

    char fx_storage[512] __attribute__((aligned(16)));

    inline void save_state(general_regs_state* task_regs,
                           interrupt_frame*    task_frame) {

        // Save floating point registers
        asm volatile(
            "fxsave64 %[fx_storage] \n\t" ::[fx_storage] "m"(fx_storage));

        // Save items from interrupt frame
        rflags = task_frame->rflags;
        rip    = task_frame->return_instruction;
        rsp    = task_frame->return_stack_pointer;

        // Save general registers
        rbp = task_regs->rbp;

        rax = task_regs->rax;
        rbx = task_regs->rbx;
        rcx = task_regs->rcx;
        rdx = task_regs->rdx;
        rsi = task_regs->rsi;
        rdi = task_regs->rdi;

        r8  = task_regs->r8;
        r9  = task_regs->r9;
        r10 = task_regs->r10;
        r11 = task_regs->r11;
        r12 = task_regs->r12;
        r13 = task_regs->r13;
        r14 = task_regs->r14;
        r15 = task_regs->r15;
    }

    inline void load_state(general_regs_state* task_regs,
                           interrupt_frame*    task_frame) {

        // Restore the floating point registers
        asm volatile(
            "fxrstor64 %[fx_storage] \n\t" ::[fx_storage] "m"(fx_storage));

        // Modify task frame
        task_frame->return_instruction   = rip;
        task_frame->return_stack_pointer = rsp;
        task_frame->rflags               = rflags;

        // Restore general registers
        task_regs->rbp = rbp;

        task_regs->rax = rax;
        task_regs->rbx = rbx;
        task_regs->rcx = rcx;
        task_regs->rdx = rdx;
        task_regs->rsi = rsi;
        task_regs->rdi = rdi;

        task_regs->r8  = r8;
        task_regs->r9  = r9;
        task_regs->r10 = r10;
        task_regs->r11 = r11;
        task_regs->r12 = r12;
        task_regs->r13 = r13;
        task_regs->r14 = r14;
        task_regs->r15 = r15;
    }

    __attribute__((noreturn)) inline void jump_into() {

        // Push address to return to task's stack
        push(rip);

        // Load floating point registers
        asm volatile("fxrstor %[fx_storage] \n\t" ::[fx_storage] "m"(fx_storage)
                     : "memory");

        // Need to push all these values onto the stack
        push_64(rsp);
        push_64(rflags);
        push_64(rbp);

        push_64(rax);
        push_64(rbx);
        push_64(rcx);
        push_64(rdx);
        push_64(rsi);
        push_64(rdi);

        push_64(r8);
        push_64(r9);
        push_64(r10);
        push_64(r11);
        push_64(r12);
        push_64(r13);
        push_64(r14);
        push_64(r15);

        // Can pop general regs into place
        POP_GENERAL_REGS();

        // Now need to pop rflags, rsp, and then use the ret to enter task
        asm volatile("pop %%rflags \n\t pop %%rsp \n\t ret \n\t");
    }

    inline void push(uint64_t value) {
        *(uint64_t*)(rsp) = value;
        rsp               = (uint64_t)((uintptr_t)rsp - sizeof(uint64_t));
    }

    inline uint64_t pop() {
        rsp = (uint64_t)((uintptr_t)rsp + sizeof(uint64_t));
        return *(uint64_t*)(rsp);
    }

    inline uint64_t peek(unsigned int index = 0) {
        return *(uint64_t*)((uint64_t)((uintptr_t)rsp
                                       + (sizeof(uint64_t) * (index + 1))));
    }
} __attribute__((aligned(16), packed));

struct process {
    processor_state saved_state;

    uint64_t pid = 0;

    struct config_t {
        bool wait_on_end = false;
    } config;

    unsigned int priority;
    unsigned int rounds;

    std_k::callable<void>* main;
    std_k::ostream         out_stream;

    allocation_manager*         task_allocation = 0;
    paging::address_space*      task_space;
    paging::page_level_4_table* lvl4_table;

    process*                parent_task;
    std_k::vector<process*> children;

    void* user_stack;
    void* kernel_stack;

    process(unsigned int target_priority, unsigned int rounds,
            std_k::callable<void>* target, process* parent = 0);

    ~process();

    static void init_wrapper(process* target);

    void prepare_wrapper() {
        saved_state.rip = (uint64_t)init_wrapper;
        saved_state.rdi = (uint64_t)this;
    }

    void* operator new(size_t size) {
        // Need to allocate on 16-byte boundary
        return aligned_alloc(size, 16);
    }
};
} // namespace threading

#endif // PINTOS_PROCESS_DEF_H

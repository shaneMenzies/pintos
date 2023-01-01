#ifndef ASM_FUNCTIONS_H
#define ASM_FUNCTIONS_H

#include <cpuid.h>
#include <stdint.h>

#define PUSH_VOLATILE_REGS() \
    asm volatile("\
    push %rax \n\t\
    push %rcx \n\t\
    push %rdx \n\t\
    push %rsi \n\t\
    push %rdi \n\t\
    push %r8 \n\t\
    push %r9 \n\t\
    push %r10 \n\t\
    push %r11 \n\t")

#define POP_VOLATILE_REGS() \
    asm volatile("\
    pop %r11 \n\t\
    pop %r10 \n\t\
    pop %r9 \n\t\
    pop %r8 \n\t\
    pop %rdi \n\t\
    pop %rsi \n\t\
    pop %rdx \n\t\
    pop %rcx \n\t\
    pop %rax \n\t")

#define PUSH_GENERAL_REGS() \
    asm volatile("\
    push %rbp \n\t\
    push %rax \n\t\
    push %rbx \n\t\
    push %rcx \n\t\
    push %rdx \n\t\
    push %rsi \n\t\
    push %rdi \n\t\
    push %r8 \n\t\
    push %r9 \n\t\
    push %r10 \n\t\
    push %r11 \n\t\
    push %r12 \n\t\
    push %r13 \n\t\
    push %r14 \n\t\
    push %r15 \n\t")

#define POP_GENERAL_REGS() \
    asm volatile("\
    pop %r15 \n\t\
    pop %r14 \n\t\
    pop %r13 \n\t\
    pop %r12 \n\t\
    pop %r11 \n\t\
    pop %r10 \n\t\
    pop %r9 \n\t\
    pop %r8 \n\t\
    pop %rdi \n\t\
    pop %rsi \n\t\
    pop %rdx \n\t\
    pop %rcx \n\t\
    pop %rbx \n\t\
    pop %rax \n\t\
    pop %rbp \n\t")

struct general_regs_state {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;

    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    uint64_t rbp;

} __attribute__((packed));
#define GENERAL_REGS_OFFSET 0x78

struct interrupt_frame {
    uint64_t return_instruction;
    uint64_t caller_segment;
    uint64_t rflags;
    uint64_t return_stack_pointer;
    uint64_t return_ss;
};
#define INTERRUPT_FRAME_OFFSET 0x28

inline uint64_t read_msr(uint32_t target_msr) {
    uint64_t low  = 0;
    uint64_t high = 0;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(target_msr));

    return (low | high);
};

inline void write_msr(uint32_t target_msr, uint64_t new_value) {
    asm volatile("wrmsr"
                 :
                 : "a"(new_value), "d"(new_value >> 32), "c"(target_msr));
};

bool fpu_init();
void set_idt(void* table, uint16_t size);
void set_gdt(void* table, uint16_t size);

inline void enable_interrupts() { asm volatile("sti"); };

inline void disable_interrupts() { asm volatile("cli"); };

inline void trigger_breakpoint() { asm volatile("int $3"); };

inline uint64_t pop_64() {
    uint64_t value;
    asm volatile("\
        pop %[value]"
                 : [value] "=r"(value));
    return value;
}

inline void push_64(uint64_t value) {
    asm volatile("\
        push %[value]" ::[value] "r"(value));
}

inline uint64_t get_rbp() {
    uint64_t value;
    asm volatile("\
        movq %%rbp, %[value]"
                 : [value] "=r"(value));
    return value;
}

inline uint64_t get_rsp() {
    uint64_t value;
    asm volatile("\
        movq %%rsp, %[value]"
                 : [value] "=r"(value));
    return value;
}

inline void set_rbp(uint64_t value) {
    asm volatile("\
        movq %[value], %%rbp" ::[value] "g"(value));
}

inline void set_rsp(uint64_t value) {
    asm volatile("\
        movq %[value], %%rsp" ::[value] "g"(value));
}

inline void jump_64(uint64_t target) {
    asm volatile("\
        jmp *%%rax" ::"a"(target));
}

inline uint64_t rd_seed() {
    uint64_t value;
    asm volatile("0: \t\n"
                 "rdseed %[value] \t\n"
        "jc 1f \t\n"
        "jmp 0b \t\n"
        "1:  \t\n"
        : [value] "=r"(value)
        :
        : "cc"
    );
    return value;
}

inline uint64_t rd_rand() {
    uint64_t value;
    asm volatile(
        "0: \t\n"
        "rdrand %[value] \t\n"
        "jc 1f \t\n"
        "jmp 0b \t\n"
        "1:  \t\n"
        : [value] "=r"(value)
        :
        : "cc"
    );
    return value;
}

inline void out_byte(unsigned char byte, uint16_t port) {
    asm volatile("movw %[port], %%dx \n\t\
         movb %[value], %%al \n\t\
         outb %%al, %%dx"
                 :
                 : [value] "g"(byte), [port] "g"(port)
                 : "rdx", "rax");
};

inline unsigned char in_byte(uint16_t port) {

    unsigned char return_value = 0;

    asm volatile("movw %[port], %%dx \n\t\
         inb %%dx, %%al \n\t\
         movb %%al, %[return_val]"
                 : [return_val] "+g"(return_value)
                 : [port] "g"(port)
                 : "rdx", "rax");

    return return_value;
};

inline void out_word(uint16_t word, uint16_t port) {
    asm volatile("movw %[port], %%dx \n\t\
         movw %[value], %%ax \n\t\
         outw %%ax, %%dx"
                 :
                 : [value] "g"(word), [port] "g"(port)
                 : "rdx", "rax");
};

inline uint16_t in_word(uint16_t port) {

    uint16_t return_value = 0;

    asm volatile("movw %[port], %%dx \n\t\
         inw %%dx, %%ax \n\t\
         movw %%ax, %[return_val]"
                 : [return_val] "+g"(return_value)
                 : [port] "g"(port)
                 : "rdx", "rax");

    return return_value;
};

inline void out_dword(uint32_t dword, uint16_t port) {
    asm volatile("movw %[port], %%dx \n\t\
         movl %[value], %%eax \n\t\
         outl %%eax, %%dx"
                 :
                 : [value] "g"(dword), [port] "g"(port)
                 : "rdx", "rax");
};

inline uint32_t in_dword(uint16_t port) {

    uint32_t return_value = 0;

    asm volatile("movw %[port], %%dx \n\t\
         inl %%dx, %%eax \n\t\
         movl %%eax, %[return_val]"
                 : [return_val] "+g"(return_value)
                 : [port] "g"(port)
                 : "rdx", "rax");

    return return_value;
};

inline void io_wait() { out_byte(0, 0x80); };

inline bool try_lock(bool* target) {
    bool result;
    asm volatile("lock cmpxchgb %[value], (%%rcx) \n\t\
                   movb %%al, %[result]"
                 : [result] "=m"(result)
                 : [target] "c"(target),
                   "a"((unsigned char)0), [value] "d"((unsigned char)1));

    return (!result);
}

inline void get_lock(bool* target) {

    while (1) {
        if (try_lock(target)) {
            return;
        } else {
            asm volatile("nop");
        }
    }
}

extern "C" {
extern volatile bool release_spinlock;
extern volatile bool spinlock_reached;
extern void (*after_spinlock_target)();
void thread_spinlock();
}

#endif

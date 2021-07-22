#ifndef ASM_FUNCTIONS_H
#define ASM_FUNCTIONS_H

#include <stdint.h>
#include <cpuid.h>

#include "io.h"

inline uint64_t read_msr(uint32_t target_msr) {
    uint64_t low = 0;
    uint64_t high = 0;
    asm volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(target_msr));

    return (low | high);
};

inline void write_msr(uint32_t target_msr, uint64_t new_value) {
    asm volatile ("wrmsr" : : "a"(new_value), "d"(new_value >> 32), "c"(target_msr));
};

inline void* get_apic_base() {
    return (void*)(read_msr(0x1b) & ~(0xfff));
};

inline uint32_t volatile* get_apic_register(uint16_t target_register) {
    return (uint32_t volatile*)((uintptr_t)get_apic_base() + target_register);
};

bool fpu_init();
void set_idt(void* table, uint16_t size);
void set_gdt(void* table, uint16_t size);

inline void enable_interrupts() {
    asm volatile ("sti");
};

inline void disable_interrupts() {
    asm volatile ("cli");
};

inline void out_byte(unsigned char byte, uint16_t port) {
    asm volatile (
        "movw %[port], %%dx \n\t\
         movb %[value], %%al \n\t\
         outb %%al, %%dx"
        :
        : [value] "g" (byte),
            [port] "g" (port)
        : "rdx", "rax"
    );
};

inline unsigned char in_byte(uint16_t port) {

    unsigned char return_value = 0;

    asm volatile (
        "movw %[port], %%dx \n\t\
         inb %%dx, %%al \n\t\
         movb %%al, %[return_val]"
        : [return_val] "+g" (return_value)
        : [port] "g" (port)
        : "rdx", "rax"
    );

    return return_value;
};

inline void out_word(uint16_t word, uint16_t port) {
    asm volatile (
        "movw %[port], %%dx \n\t\
         movw %[value], %%ax \n\t\
         outw %%ax, %%dx"
        :
        : [value] "g" (word),
            [port] "g" (port)
        : "rdx", "rax"
    );
};

inline uint16_t in_word(uint16_t port) {

    uint16_t return_value = 0;

    asm volatile (
        "movw %[port], %%dx \n\t\
         inw %%dx, %%ax \n\t\
         movw %%ax, %[return_val]"
        : [return_val] "+g" (return_value)
        : [port] "g" (port)
        : "rdx", "rax"
    );

    return return_value;
};

inline void out_dword(uint32_t dword, uint16_t port) {
    asm volatile (
        "movw %[port], %%dx \n\t\
         movl %[value], %%eax \n\t\
         outl %%eax, %%dx"
        :
        : [value] "g" (dword),
            [port] "g" (port)
        : "rdx", "rax"
    );
};

inline uint32_t in_dword(uint16_t port) {

    uint32_t return_value = 0;

    asm volatile (
        "movw %[port], %%dx \n\t\
         inl %%dx, %%eax \n\t\
         movl %%eax, %[return_val]"
        : [return_val] "+g" (return_value)
        : [port] "g" (port)
        : "rdx", "rax"
    );

    return return_value;
};

inline void io_wait() {
    out_byte(0, 0x80);
};

#endif
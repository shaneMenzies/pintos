/**
 * @file asm_functions.cpp
 * @author Shane Menzies
 * @brief Assembly based functions
 * @date 04/20/21
 *
 *
 */

#include "libk/asm.h"

#include "time/timer.h"

bool fpu_init() {

    uint16_t fpu_status = 1;

    uint64_t cr0_and = ~((1 << 2) | (1 << 3));
    uint64_t cr0_or  = (1 << 1);

    uint64_t cr4_or = ((1 << 9) | (1 << 10) | (1 << 18));

    asm volatile(
        "movw $0xffff, %[fpu_status] \n\t\
         movq %%cr0, %%rdx \n\t\
         and %[cr0_and], %%rdx \n\t\
         or %[cr0_or], %%rdx \n\t\
         movq %%rdx, %%cr0 \n\t\
         fninit \n\t\
         fnstsw %[fpu_status] \n\t\
         movq %%cr4, %%rdx \n\t\
         or %[cr4_or], %%rdx \n\t\
         movq %%rdx, %%cr4 \n\t\
         push %%rax \n\t\
         push %%rcx \n\t\
         push %%rdx \n\t\
         xor %%rcx, %%rcx \n\t\
         xgetbv \n\t\
         or $7, %%eax \n\t\
         xsetbv \n\t\
         pop %%rdx \n\t\
         pop %%rcx \n\t\
         pop %%rax \n\t"
        : [fpu_status] "+m"(fpu_status)
        : [cr0_and] "g"(cr0_and), [cr0_or] "g"(cr0_or), [cr4_or] "g"(cr4_or)
        : "rdx");

    if (fpu_status == 0) {
        return true;
    } else {
        return false;
    }
}

void set_idt(void* table, uint16_t size) {

    struct {
        uint16_t length;
        void*    base;
    } __attribute__((packed)) IDTR = {size, table};

    asm volatile("lidt %0" : : "m"(IDTR));
}

void set_gdt(void* table, uint16_t size) {

    struct {
        uint16_t length;
        void*    base;
    } __attribute__((packed)) GDTR = {size, table};

    asm volatile("lgdtq %[gdtr] \n\t" ::[gdtr] "m"(GDTR) :);
}

void set_tss(uint16_t selector) {
    asm volatile("ltrw %[ist] \n\t" ::[ist] "a"(selector) :);
}

extern "C" {
volatile bool release_spinlock  = false;
volatile bool spinlock_reached  = false;
void (*after_spinlock_target)() = (void (*)()) & thread_spinlock;

void thread_spinlock() {
    // Set flag that this thread has reached the spinlock
    spinlock_reached = true;

    // Wait for the spinlock to be released by the boot processor
    while (1) {
        if (release_spinlock) {
            break;
        } else {
            asm volatile("nop");
        }
    }

    // Reset the flags for the next thread
    release_spinlock = false;
    spinlock_reached = false;

    // Go to new target code
    after_spinlock_target();
}
}

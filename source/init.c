/**
 * @file init.c
 * @author Shane Menzies
 * @brief Initializes various requirements required by the main kernel
 * @date 02/24/21
 * 
 * 
 */

#include "multiboot.h"
#include "memory.h"
#include "display.h"
#include "terminal.h"
#include "kernel.h"

extern uint32_t return_ebx();

/**
 * @brief Error function as result from the call of a purely
 *        virtual function
 * 
 */
void __cxa_pure_virtual() {
    // TODO: ERROR: Purely virtual function call
}

/**
 * @brief Main Initialization function, runs even before the
 *        global constructor function, _init, in crti.s
 * 
 */
void kernel_init() {
    struct mb_info* mb_addr = (struct mb_info*) return_ebx();

    memory_init(mb_addr);

    framebuffer_init(mb_addr);
}
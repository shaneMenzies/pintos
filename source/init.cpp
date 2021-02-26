/**
 * @file init.c
 * @author Shane Menzies
 * @brief Initializes various requirements required by the main kernel
 * @date 02/24/21
 * 
 * 
 */



#include "kernel.h"

#include "multiboot.h"
#include "display.h"

extern "C" {

extern void _init();

}

/*
uint32_t return_ebx() {
    uint32_t ebx;
    asm("movl %%ebx, %[ebxVal]\n" "ret" : [ebxVal] "=rm" (ebx));
    return ebx;
}
*/

/**
 * @brief Error function as result from the call of a purely
 *        virtual function
 * 
 */
void __cxa_pure_virtual() {
    // TODO: ERROR: Purely virtual function call
}

/**
 * @brief Main Initialization function, runs before kernel_main
 * 
 */
void kernel_init() {
    return;
}
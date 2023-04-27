/**
 * @file process_interface.cpp
 * @author Shane Menzies
 * @brief
 * @date 12/21/22
 *
 *
 */

#include "process_interface.h"

#include "interrupts/interrupts.h"
#include "libk/asm.h"

namespace this_process {

void yield() {
    // Manually trigger the yield interrupt
    asm volatile("int $0xa1");
}

void sleep(double seconds) {}

} // namespace this_process

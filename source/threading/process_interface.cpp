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

void sleep(double seconds) {
    timer<>::timestamp target_time
        = get_timer()->now() + get_timer()->convert_sec(seconds);

    // Lazy timing check uses value on top of stack
    push_64((uint64_t)target_time);
    common_region::current_process->waiting = threading::lazy_check;
    yield();
}

} // namespace this_process

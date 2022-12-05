/**
 * @file apic.cpp
 * @author Shane Menzies
 * @brief
 * @date 7/3/22
 *
 *
 */

#include "apic.h"

#include "libk/asm.h"
#include "system/kernel.h"

namespace current_apic {

volatile uint32_t* apic_base = (volatile uint32_t*)0xfee00000;
uint32_t           determine_apic_tick_rate() {

    // Divide bus frequency by 8
    get_register(0x3e0) = 0b010;

    // Make sure interrupts are masked
    get_register(0x320) = 0x10000;

    // Set apic timer initial count to -1
    get_register(0x380) = ~(0);

    // Sleep for 0.1 seconds
    sys_int_timer->sleep(0.1);

    // Determine how many ticks occurred in that time period
    uint32_t ticks = get_register(0x390);
    ticks          = (~(0)) - ticks;
    ticks *= 10;
    return ticks;
}

} // namespace current_apic

/**
 * @file timer.cpp
 * @author Shane Menzies
 * @brief Organization of timed tasks to be run repeatedly
 * @date 04/06/21
 *
 *
 */

#include "timer.h"

#include "hpet.h"
#include "interrupts/interrupts.h"
#include "io/io.h"
#include "libk/functional.h"
#include "memory/p_memory.h"
#include "system/acpi.h"

constexpr unsigned long int PIT_OSC_RATE = 1193182;

void legacy_timer_init(unsigned long int rate) {

    // Legacy PIT setup

    // Determine the 16-bit divider for the PIT
    uint16_t divider;
    if (rate <= 19) {
        divider = 0;
    } else {
        divider = PIT_OSC_RATE / rate;
    }

    // Make the PIT command (channel 0, lo/hi byte access, mode 2, binary
    // mode)
    unsigned char command = 0b00110110;
    out_byte(command, PIT_CMD);

    // send the divider in two parts (low first)
    out_byte((unsigned char)(divider & 0x0ff), PIT_CH0);
    out_byte((unsigned char)((divider & 0xff00) >> 8), PIT_CH0);

    // Set appropriate interrupt
    interrupts::set_direct_interrupt(
        OFFSET_1 + 0, interrupts::gate_type::INT_GATE_64, interrupts::irq_0);
}

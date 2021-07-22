#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdarg.h>
#include <stdint.h>

#include "terminal_def.h"
#include "p_memory.h"
#include "display.h"
#include "libk.h"
#include "io.h"
#include "commands.h"

extern visual_terminal* boot_terminal;
extern visual_terminal* active_terminal;

void draw_active_cursor();

/**
 * @brief Converts a number into characters in the target char array
 * 
 * @param target_buffer     Char buffer for output to be placed in
 * @param number            Number to convert
 * @param base              Base to interpret number as
 * @return uint16_t         Chars placed
 */
template<typename T>
uint16_t stringify(char* target_buffer, T number, uint8_t base) {

    uint16_t index = 0;
    // Allocate some memory to temporarily store the digits of the number in
    char temp_string[64];

    // Flag to set if number was negative
    bool negative = false;

    // If the number is negative (and signed), put a - in front and make it positive
    if (is_signed<T>() && number < 0) {
        negative = true;
        number = -number;
    }

    // Loop through placing digits until the number is gone
    while (1) {
        // Get the digit
        char digit = number % base;
        number /= base;

        // Convert digit to corresponding ascii code
        if (digit < 10) {
            // Numerical characters
            digit += 48;
        } else {
            // Alphabetical characters
            // (87 instead of 97, simplified from (digit - 10) + 97)
            digit += 87;
        }

        // Place the digit
        temp_string[index] = digit;

        // Break if number is 0
        if (number <= 0) {
            break;
        } else {
            // Else increment the index
            index++;
        }

    }

    if (negative) {
        temp_string[++index] = '-';
    }

    // New index for target_buffer
    uint16_t t_index = 0;

    // temp_string now contains the digits, but they're backwards,
    // so work through them backwards placing them in the target_buffer
    while (1) {

        target_buffer[t_index] = temp_string[index];

        if (index == 0) {
            break;
        }

        t_index += 1;
        index -= 1;
    }

    // Return the number of chars placed
    return (t_index + 1);
}

#endif
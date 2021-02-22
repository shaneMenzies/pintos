/**
 * @file terminal.c
 * @author Shane Menzies
 * @brief Functions and structures for terminal functionality
 * @date 02/14/21
 * 
 * 
 */

#include "terminal.h"

#include "memory.h"
#include "display.h"
#include "libk.h"

#include <stdarg.h>

/**
 * @brief Create a new terminal object
 * 
 * @param fg_color          Default foreground color 
 * @param bg_color          Default background color
 * @param ega_attributes    Default EGA colors
 * @param max_chars         Maximum characters for the terminal
 * @return terminal*        Pointer to the new terminal
 */
terminal* create_terminal(uint32_t fg_color, uint32_t bg_color, uint8_t ega_attributes, unsigned int max_chars) {
    terminal* new_terminal = (terminal*) malloc(sizeof(terminal));

    new_terminal->default_ega = ega_attributes;
    new_terminal->default_fg = fg_color;
    new_terminal->default_bg = bg_color;
    new_terminal->max_chars = max_chars;
    new_terminal->index = 0;

    new_terminal->text[0] = '\0';

    return new_terminal;
}

/**
 * @brief Writes the string to the target terminal
 * 
 * @param target_terminal   Pointer to terminal to write to
 * @param string            String to write (null-terminated)
 */
void terminal_write(terminal* target_terminal, char* string) {

    unsigned int string_index = 0;
    unsigned int terminal_index = target_terminal->index;
    unsigned int max_chars = target_terminal->max_chars;
    char target_char = string[string_index];

    // Copy all of the characters in the string into the terminal buffer
    while(target_char != '\0') {

        target_terminal->text[terminal_index] = target_char;
        string_index++;
        terminal_index++;

        if (terminal_index >= max_chars) {
            terminal_index = 0;
        }

       target_char = string[string_index];
    }

    // Adds newline and null termination 
    target_terminal->text[terminal_index] = '\n';
    terminal_index++;
    target_terminal->text[terminal_index] = '\0';

    target_terminal->index = terminal_index;
}

/**
 * @brief Prints the entire contents of that target terminal to the screen
 * 
 * @param target_terminal   Pointer to the target terminal
 */
void terminal_print(terminal* target_terminal) {
    if (fb.direct_color) {
        fb_puts(0, 0, target_terminal->text, target_terminal->default_fg, 
                target_terminal->default_bg);
    } else {    
        ega_puts(0, 0, target_terminal->default_ega, target_terminal->text);
    }
}

/**
 * @brief Converts a number into characters in the target char array
 * 
 * @param target_buffer     Char buffer for output to be placed in
 * @param number            Number to convert
 * @param base              Base to interpret number as
 * @return uint16_t         Chars placed
 */
uint16_t stringify(char* target_buffer, int number, uint8_t base) {

    uint16_t index = 0;
    // Allocate some memory to temporarily store the digits of the number in
    char* temp_string = malloc(64);

    // Flag to set if number was negative
    bool negative = false;

    // If the number is negative, put a - in front and make it positive
    if (number < 0) {
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

    // Free the allocated memory
    free(temp_string);

    // Return the number of chars placed
    return (t_index + 1);
}

/**
 * @brief Writes the formatted string to the target terminal,
 *        following basic printf syntax
 * 
 * @param target_term   Terminal to write to
 * @param format        Format string
 * @param ...           Arguments for formatting, in order
 */
void tprintf(terminal* target_term, char* format, ...) {

    // Start the optional arguments
    va_list args;
    va_start(args, format);

    // Allocate memory for building the formatted string
    char* build = (char*)malloc(1024);

    // Index for format string
    int f_index = 0;

    // Index for build string
    int b_index = 0;

    while (1) {
        char target_char = format[f_index];

        // Catch if char is a %, then get next char, and act on it
        if (target_char == '%') {

            f_index++;
            target_char = format[f_index];

            switch (target_char) {
                case 'd':
                    /* fall through */
                case 'i':
                    // SIGNED INTEGER TO DECIMAL CHARS
                    b_index += (stringify(&build[b_index], 
                                va_arg(args, int), 10));
                    break;

                case 'u':
                    // UNSIGNED INTEGER TO DECIMAL CHARS
                    b_index += stringify(&build[b_index], 
                                (int)va_arg(args, unsigned int), 10);
                    break;

                case 'o':
                    // UNSIGNED INTEGER TO OCTAL CHARS
                    b_index += stringify(&build[b_index], 
                                (int)va_arg(args, unsigned int), 8);
                    break;

                case 'x':
                    // UNSIGNED INTEGER TO HEX CHARS
                    b_index += stringify(&build[b_index], 
                                (int)va_arg(args, unsigned int), 16);
                    break;

                case 'c':
                    // PRINT THE CHAR
                    build[b_index] = (char) va_arg(args, int);
                    b_index++;
                    break;

                case 's': {
                        // A STRING
                        uint16_t temp_index = 0;
                        char* string = va_arg(args, char*);

                        // Loop until null terminator encountered
                        while (1) {

                            char string_char = string[temp_index];

                            if (string_char == '\0') {
                                break;
                            }

                            build[b_index] = string_char;
                            b_index++;
                            temp_index++;
                        }
                    }
                    break;

                case 'p':
                    // POINTER TO HEX DIGITS
                    b_index += stringify(&build[b_index], 
                                (int)va_arg(args, int*), 16);
                    break;

                case 'n':
                    // STORE b_index TO POINTER PROVIDED
                    *(va_arg(args, int*)) = b_index;
                    break;

                case '%':
                    // PRINT A %
                    build[b_index] = '%';
                    b_index++;
                    break;

                default:
                    break;
            }

        // By default just copy the character over
        } else {
            build[b_index] = target_char;
            b_index++;

            // Null termination ends loop
            if (target_char == '\0') {
                break;
            }
        }

        // Increment format index
        f_index++;
    }

    va_end(args);

    terminal_write(target_term, build);

    // Deallocate the previously allocated memory
    free((void*)build);
}

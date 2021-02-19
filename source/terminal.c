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

/**
 * @brief Create a terminal object
 * 
 * @param fg_color          Default foreground color 
 * @param bg_color          Default background color
 * @param ega_attributes    Default EGA colors
 * @param max_chars         Maximum characters for the terminal
 * @return terminal         New terminal structure ready to be written to
 */
terminal create_terminal(uint32_t fg_color, uint32_t bg_color, uint8_t ega_attributes, unsigned int max_chars) {
    terminal* new_terminal = (terminal*) malloc(sizeof(terminal));

    new_terminal->default_ega = ega_attributes;
    new_terminal->default_fg = fg_color;
    new_terminal->default_bg = bg_color;
    new_terminal->max_chars = max_chars;
    new_terminal->index = 0;

    new_terminal->text[0] = '\0';

    return *new_terminal;
}

/**
 * @brief Writes the string to the target terminal
 * 
 * @param target_terminal   Pointer to terminal to write to
 * @param string            String to write (null-terminated)
 */
void terminal_write(terminal* target_terminal, char string[]) {

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

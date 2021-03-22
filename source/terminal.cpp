/**
 * @file terminal.cpp
 * @author Shane Menzies
 * @brief Functions and structures for terminal functionality
 * @date 02/14/21
 * 
 * 
 */

#include "terminal.h"

visual_terminal* boot_terminal;
visual_terminal* active_terminal;

/* #region terminal */

terminal::terminal(size_t text_size) {

    // Allocate the text buffer
    start = (char*)malloc(text_size);
    next = start;
    end = (char*)((uintptr_t)start + text_size);

    // Clear out the keyboard buffer
    kb_clear();
}

terminal::~terminal() {

    // Clear text buffer
    while (end > start) {
        *end = 0;
        end--;
    }

    // Clear keyboard buffer
    kb_clear();

    // Free the text buffer
    free(start);
}

void terminal::kb_clear() {
    for (int index = 0; index < KB_BUF_SIZE; index++) {
        keyboard[index] = 0;
    }
}

void terminal::kb_append_c(char new_char) {
    keyboard[kb_index++] = new_char;
    keyboard[kb_index] = '\0';

    if (kb_index >= KB_BUF_SIZE)
        kb_index = 0;
}

void terminal::kb_append_s(const char* string) {
    
    unsigned int string_index = 0;

    while (1) {
        char target_char = string[string_index];
        keyboard[kb_index++] = target_char;

        if (kb_index >= KB_BUF_SIZE)
            kb_index = 0;
        
        if (target_char == '\0')
            break;
    }
}

void terminal::write(const char* string) {
    unsigned int string_index = 0;
    char target_char;

    // Copy all of the characters in the string into the terminal buffer
    while(1) {

        target_char = string[string_index];

        *next = target_char;

        if (target_char == '\0')
            break;
        
        next++;
        string_index++;

        // If reached the end of the buffer, loop back
        if (next > end) {
            next = start;
        }
    }
}

void terminal::tprintf(const char* format, ...) {

    // Start the optional arguments
    va_list args;
    va_start(args, format);

    // Allocate memory for building the formatted string
    char* build = (char*)malloc(1024);

    vprintf(build, format, args);
    va_end(args);

    this->write(build);

    // Deallocate the previously allocated memory
    free((void*)build);
}

void terminal::clear() {
    next = start;
    *next = '\0';
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
    char* temp_string = (char*) malloc(64);

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

/* #endregion */

/* #region visual_terminal */

visual_terminal::visual_terminal(size_t text_size, uint32_t fg, uint32_t bg, 
    uint8_t ega, unsigned int target_lines) : terminal(text_size), 
    default_fg(fg), default_bg(bg), default_ega(ega), target_lines(target_lines) {

    // How many lines can be displayed
    v_height = fb.height / glyph_height;

    // Chars and bytes in each line
    v_width = fb.width / glyph_width;
    v_char_pitch = fb.pitch * glyph_height;

    // Allocate space for the visual buffer
    size_t v_size = (size_t)(v_height * v_width);
    v_buffer_start = (char*)malloc(v_size);
    *v_buffer_start = '\0';
    v_buffer_next = v_buffer_start;
    v_buffer_end = v_buffer_start + v_size;
}

void visual_terminal::tprintf(const char* format, ...) {

    // Start the optional arguments
    va_list args;
    va_start(args, format);

    // Allocate memory for building the formatted string
    char* build = (char*)malloc(1024);

    vprintf(build, format, args);
    va_end(args);

    this->write(build);

    // Deallocate the previously allocated memory
    free((void*)build);
}

void visual_terminal::write(const char* string) {

    unsigned int string_index = 0;
    char target_char;

    // Copy all of the characters in the string into the terminal buffer
    while(1) {

        target_char = string[string_index];

        *next = target_char;
        *v_buffer_next = target_char;

        if (target_char == '\0') {
            break;
        } else if (target_char == '\n') {
            cur_y++;
            cur_x = 0;
        } else if (cur_x == v_width) {
            cur_y++;
            cur_x = 0;
        } else {    
            cur_x++;
        }
        
        next++;
        v_buffer_next++;
        string_index++;

        // If reached the end of the buffer, loop back
        if (next > end) {
            next = start;
        }
        if (v_buffer_next > v_buffer_end) {
            v_buffer_next = v_buffer_start;
            cur_y = 0;
            cur_x = 0;
        }
    }

    // Determine what shift needs to be made to the visual buffer
    if (cur_y > target_lines) {
        // How many lines to shift?
        int line_shift = (cur_y - target_lines) + 4;

        // Determine ddress to cut from start of buffer
        char* end_cut = v_buffer_start;
        int lines = 0;
        unsigned int counter = 0;
        while (1) {

            if (*end_cut == '\n') {
                lines++;
                counter = 0;
            }

            if (counter == v_width) {
                lines++;
                counter = 0;
            }

            end_cut++;
            counter++;

            if (lines == line_shift)
                break;
        }

        // Cut that space from the buffer
        memcpy(v_buffer_start, end_cut, (size_t)((uintptr_t)v_buffer_end - (uintptr_t)end_cut));
        cur_y -= line_shift;
        v_buffer_next -= (size_t)((uintptr_t)end_cut - (uintptr_t)v_buffer_start);
        fb_blank(default_bg);
    }
}

void visual_terminal::clear() {
    fb_blank(default_bg);
    v_buffer_next = v_buffer_start;
    *v_buffer_next = '\0';
    cur_y = 0;
    cur_x = 0;
}

void visual_terminal::show() {
    if (fb.direct_color)
        fb_puts(0, 0, v_buffer_start, default_fg, default_bg);
    else
        ega_puts(0, 0, default_ega, v_buffer_start);
}

void visual_terminal::show(uint32_t fg_color, uint32_t bg_color, uint8_t ega_attributes) {
    if (fb.direct_color)
        fb_puts(0, 0, v_buffer_start, fg_color, bg_color);
    else
        ega_puts(0, 0, ega_attributes, v_buffer_start);
}


/* #endregion */

    

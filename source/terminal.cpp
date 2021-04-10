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

terminal::terminal(size_t text_size) : handler() {

    // Allocate the text buffer
    start = (char*)malloc(text_size);
    next = start;
    end = (char*)((uintptr_t)start + text_size);


    // Set up the special actions for the command-line
    handler.set_signal(0xff, kernel::send_command);
    handler.set_action('\n', keyboard::key_action(keyboard::SEND_SIGNAL, 0xff));
}

terminal::~terminal() {

    // Clear text buffer
    while (end > start) {
        *end = 0;
        end--;
    }

    // Free the text buffer
    free(start);
}

void terminal::set_handler(keyboard::kb_handler new_handler) {

    handler = new_handler;
}

inline void terminal::send_key(char character) {
    handler.run_action(character);
    write_c(character);
}

void terminal::write_c(const char character) {

    *next = character;

    next++;
    if (next > end)
            next = start;

    *next = '\0';
}

void terminal::write_s(const char* string) {
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

    this->write_s(build);

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
    uint8_t ega, double target_fill) : terminal(text_size), 
    fb(), cursor(fb.char_width, fb.char_height), 
    default_fg(fg), default_bg(bg), default_ega(ega), 
    target_fill(target_fill) {

    target_height = (target_fill * fb.info.height);
    target_height -= (target_height % fb.char_height);

    scroll_shift = fb.info.height - target_height;
    scroll_shift -= (scroll_shift % fb.char_height);
}

void visual_terminal::tprintf(const char* format, ...) {

    // Start the optional arguments
    va_list args;
    va_start(args, format);

    // Allocate memory for building the formatted string
    char* build = (char*)malloc(1024);

    vprintf(build, format, args);
    va_end(args);

    this->write_s(build);

    // Deallocate the previously allocated memory
    free((void*)build);
}

void visual_terminal::write_c(const char character) {

    *next = character;

    next++;
    if (next > end)
            next = start;

    *next = '\0';

    // Print the string to the framebuffer
    fb.draw_c(x_pos, y_pos, character, default_fg, default_bg, default_ega);

    // Determine what shift needs to be made to the visual buffer
    if (y_pos > target_height) {
        // Calculate how much to cut out
        unsigned int lines = (y_pos - target_height) + scroll_shift;
        size_t line_shift = lines * fb.info.pitch;

        // Cut that many lines from the buffer
        void* end_cut = (void*)((uintptr_t)fb.info.address + line_shift);
        memcpy(fb.info.address, end_cut, (fb.info.size - line_shift - 1));
        fill_mem((void*)((uintptr_t)fb.info.end - line_shift), line_shift, default_bg);
        y_pos -= lines;
    }

    update();
}

void visual_terminal::write_s(const char* string) {

    unsigned int string_index = 0;
    char target_char;

    // Copy all of the characters in the string into the terminal buffer
    while(1) {

        target_char = string[string_index];

        // Write into the text buffer
        *next = target_char;

        if (target_char == '\0') {
            break;
        } 

        // Increase indexes
        next++;
        string_index++;

        // If reached the end of the buffer, loop back
        if (next > end) {
            next = start;
        }
    }

    // Print the string to the framebuffer
    fb.draw_s(x_pos, y_pos, string, default_fg, default_bg, default_ega);

    // Determine what shift needs to be made to the visual buffer
    if (y_pos > target_height) {
        // Calculate how much to cut out
        unsigned int lines = (y_pos - target_height) + scroll_shift;
        size_t line_shift = lines * fb.info.pitch;

        // Cut that many lines from the buffer
        void* end_cut = (void*)((uintptr_t)fb.info.address + line_shift);
        memcpy(fb.info.address, end_cut, (fb.info.size - line_shift - 1));
        fill_mem((void*)((uintptr_t)fb.info.end - line_shift), line_shift, default_bg);
        y_pos -= lines;
    }

    update();
}

void visual_terminal::draw_cursor() {

    if (!cursor_active) return;

    static uint32_t color = 0x0;
    color = ~color;

    cursor.blank(color);
    cursor.show(x_pos, y_pos);
}

void draw_active_cursor() {
    active_terminal->draw_cursor();
}

void visual_terminal::clear() {
    fb.blank(default_bg);
    x_pos = 0;
    y_pos = 0;
}

/* #endregion */

    

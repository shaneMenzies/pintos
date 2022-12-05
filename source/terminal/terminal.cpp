/**
 * @file terminal.cpp
 * @author Shane Menzies
 * @brief Functions and structures for terminal functionality
 * @date 02/14/21
 *
 *
 */

#include "terminal.h"

#include "commands.h"
#include "io/io.h"
#include "io/keyboard.h"
#include "libk/cstring.h"
#include "libk/misc.h"
#include "memory/p_memory.h"
#include "terminal_def.h"

visual_terminal* boot_terminal;
visual_terminal* active_terminal;

/* #region terminal */

terminal::terminal(size_t text_size)
    : handler() {

    // Allocate the text buffer
    start = (char*)malloc(text_size);
    next  = start;
    end   = (char*)((uintptr_t)start + text_size);

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
    if (next > end) next = start;

    *next = '\0';
}

void terminal::write_s(const char* string) {
    unsigned int string_index = 0;
    char         target_char;

    // Copy all of the characters in the string into the terminal buffer
    while (1) {

        target_char = string[string_index];

        *next = target_char;

        if (target_char == '\0') break;

        next++;
        string_index++;

        // If reached the end of the buffer, loop back
        if (next > end) { next = start; }
    }
}

void terminal::tprintf(const char* format, ...) {

    // Start the optional arguments
    va_list args;
    va_start(args, format);

    vtprintf(format, args);

    va_end(args);
}

void terminal::vtprintf(const char* format, va_list args) {

    // Allocate memory for building the formatted string
    char* build = (char*)malloc(1024);

    std_k::vprintf(build, format, args);
    write_s(build);

    // Deallocate the previously allocated memory
    free((void*)build);
}

void terminal::clear() {
    next  = start;
    *next = '\0';
}

/* #endregion */

/* #region visual_terminal */

visual_terminal::visual_terminal(size_t text_size, uint32_t fg, uint32_t bg,
                                 uint8_t ega, double target_fill)
    : terminal(text_size)
    , fb()
    , cursor(fb.char_width, fb.char_height)
    , default_fg(fg)
    , default_bg(bg)
    , default_ega(ega)
    , target_fill(target_fill) {

    target_height = (target_fill * fb.info.height);
    target_height -= (target_height % fb.char_height);

    scroll_shift = fb.info.height - target_height;
    scroll_shift -= (scroll_shift % fb.char_height);

    io_write_s("\rCreating new Visual terminal.\r\n", COM_1);

    char string_buffer[2048];
    char format[] = "Framebuffer info:\r\nHeight: %u\r\n\
                        Width: %u\r\nChar Height: %u\r\n\
                        Char Width: %u\r\nTarget Height: %u\r\n\
                        Scroll Shift: %u\r\n";

    std_k::printf(string_buffer, format, fb.info.height, fb.info.width,
                  fb.char_height, fb.char_width, target_height, scroll_shift);
    io_write_s(string_buffer, COM_1);
}

void visual_terminal::write_c(const char character) {

    *next = character;

    next++;
    if (next > end) next = start;

    *next = '\0';

    // Print the string to the framebuffer
    fb.draw_c(x_pos, y_pos, character, default_fg, default_bg, default_ega);

    // Determine what shift needs to be made to the visual buffer
    if (y_pos > target_height) {
        // Calculate how much to cut out
        unsigned int lines      = (y_pos - target_height) + scroll_shift;
        size_t       line_shift = lines * fb.info.pitch;

        // Cut that many lines from the buffer
        void* end_cut = (void*)((uintptr_t)fb.info.address + line_shift);
        std_k::memcpy(fb.info.address, end_cut,
                      (fb.info.size - line_shift - 1));
        std_k::memset((void*)((uintptr_t)fb.info.end - line_shift), default_bg,
                      line_shift);
        y_pos -= lines;
    }

    update();
}

void visual_terminal::write_s(const char* string) {

    unsigned int string_index = 0;
    char         target_char;

    // Copy all of the characters in the string into the terminal buffer
    while (1) {

        target_char = string[string_index];

        // Write into the text buffer
        *next = target_char;
        if (target_char == '\0') { break; }

        // Draw to the framebuffer
        fb.draw_c(x_pos, y_pos, target_char, default_fg, default_bg,
                  default_ega);

        // Determine what shift needs to be made to the visual buffer
        if (y_pos > target_height) {
            // Calculate how much to cut out
            unsigned int lines      = (y_pos - target_height) + scroll_shift;
            size_t       line_shift = lines * fb.info.pitch;

            // Cut that many lines from the buffer
            void* end_cut = (void*)((uintptr_t)fb.info.address + line_shift);
            std_k::memcpy(fb.info.address, end_cut,
                          (fb.info.size - line_shift - 1));
            std_k::memset((void*)((uintptr_t)fb.info.end - line_shift),
                          default_bg, line_shift);
            y_pos -= lines;
        }

        // Increase indexes
        next++;
        string_index++;

        // If reached the end of the buffer, loop back
        if (next > end) { next = start; }
    }

    update();
}

void visual_terminal::draw_cursor(int state) {

    if (!cursor_active) return;

    static uint32_t color = 0x0;
    if (state < 0) {
        color = ~color;
    } else if (state > 0) {
        color = ~(0);
    } else {
        color = 0;
    }

    // cursor.blank(color);
    cursor.draw_rect_fill(0, 0, cursor.info.width - 1, cursor.info.height - 1,
                          color);
    cursor.show(x_pos, y_pos);
}

void draw_active_cursor() { active_terminal->draw_cursor(); }

void visual_terminal::clear() {
    fb.blank(default_bg);
    x_pos = 0;
    y_pos = 0;
}

/* #endregion */

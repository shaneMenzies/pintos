/**
 * @file display.cpp
 * @author Shane Menzies
 * @brief Functions dealing with basic display functions around the 
 *        framebuffer or vga text console
 * @date 02/08/21
 * 
 * 
 */

#include "display.h"

#include "memory.h"
#include "multiboot.h"
#include "error.h"
#include "libk.h"
#include "kernel.h"

bool fb_initialized = false;

struct fb_info fb;

/**
 * @brief Sets up the framebuffer info based on what multiboot provides
 * 
 * @param mb_addr   Pointer to the multiboot info structure
 */
void framebuffer_init(struct mb_info* mb_addr) {
    // Check to make sure that multiboot info has been processed
    if (mb_addr->flags & FRAMEBUFFER_FLAG) {
        fb_initialized = true;

        fb.address = (void*)((unsigned int)(mb_addr->framebuffer_addr & POINTER_MASK));
        fb.pitch = mb_addr->framebuffer_pitch;
        fb.width = mb_addr->framebuffer_width;
        fb.height = mb_addr->framebuffer_height;
        fb.depth = mb_addr->framebuffer_bpp;
        fb.pixel_size = mb_addr->framebuffer_pitch / mb_addr->framebuffer_width;
        fb.size = (size_t)(fb.pitch * fb.height);
        fb.end = (void*)((uintptr_t)fb.address + fb.size);

        // Set the correct info into the framebuffer info structure
        if (mb_addr->framebuffer_type == 0) {
            fb.palette_color = true;

            for (int i = 0; i < 4; i++) {
                fb.palette_addr |= (uint32_t)(mb_addr->color_info[i] << (8*i));
            }
            fb.palette_num = (uint16_t)((mb_addr->color_info[4] << 8) 
                                        | mb_addr->color_info[5]);
            fb.ega_text = false;
            fb.direct_color = false;
        } else if (mb_addr->framebuffer_type == 1) {
            fb.direct_color = true;

            fb.red_pos = mb_addr->color_info[0];
            fb.red_mask = mb_addr->color_info[1];
            fb.green_pos = mb_addr->color_info[2];
            fb.green_mask = mb_addr->color_info[3];
            fb.blue_pos = mb_addr->color_info[4];
            fb.blue_mask = mb_addr->color_info[5];

            fb.ega_text = false;
            fb.palette_color = false;
        } else {
            fb.ega_text = true;
            fb.direct_color = false;
            fb.palette_color = false;
        }

    } else {
        fb.ega_text = true;
        fb.direct_color = false;
        fb.palette_color = false;
    }
}

/**
 * @brief Generates an EGA/VGA attribute byte from the inputted 4-bit IRGB
 *        foreground color, 3-bit RGB background color, and bool value to
 *        make the character blink
 * 
 * @param fg_color      4-bit color-code, IRGB (intensity, red, green, blue)
 * @param bg_color      4-bit color-code for background color
 * @return uint8_t      EGA/VGA Attribute value
 */
inline uint8_t ega_attributes(uint8_t fg_color, uint8_t bg_color) {
    return (uint8_t)(((bg_color & 0b1111) << 4) | (fg_color & 0b1111));
}

v_fb::v_fb() {

    // By default copy from the real framebuffer
    info = fb;
    
    // Allocate space for new virtual framebuffer
    info.address = malloc(info.size);
    info.end = (void*)((uintptr_t)info.address + info.size);

    // Set up font info
    if (info.direct_color) {
        char_height = font->char_height;
        char_width = font->char_width;
    } else {
        char_height = 1;
        char_width = 1;
    }
    char_pitch = char_height * info.pitch;

    blank(0);
}

v_fb::v_fb(uint32_t width, uint32_t height) {

    // Copy most info from the real framebuffer
    info = fb;

    // Change certain properties to the given parameters
    info.width = width;
    info.height = height;

    info.pitch = info.pixel_size * width;
    info.size = (size_t)(info.pitch * info.height);

    // Allocate space for new virtual framebuffer
    info.address = malloc(info.size);
    info.end = (void*)((uintptr_t)info.address + info.size);

    // Set up font info
    if (info.direct_color) {
        char_height = font->char_height;
        char_width = font->char_width;
    } else {
        char_height = 1;
        char_width = 1;
    }
    char_pitch = char_height * info.pitch;

    blank(0);
}

v_fb::~v_fb() {

    // Free the virtual framebuffer memory
    free(info.address);
}

void v_fb::set_font(Font* new_font) {

    font = new_font;

    // Set up font info
    if (info.direct_color) {
        char_height = font->char_height;
        char_width = font->char_width;
    } else {
        char_height = 1;
        char_width = 1;
    }
    char_pitch = char_height * info.pitch;
}

Font* v_fb::get_font() {
    return font;
}

/* #region EGA FUNCTIONS */

/**
 * @brief Function to put a certain character on the screen at
 *        the specified position, with the specified attributes
 * 
 * @param x                 X position (columns)
 * @param y                 Y position (rows)
 * @param ega_attributes    EGA/VGA Attributes
 * @param character         Character to be printed
 */
void v_fb::ega_putc(uint32_t& x, uint32_t& y, uint8_t ega_attributes, const char character) {
    if(fb_initialized && info.ega_text) {

        uint16_t* target_address = (uint16_t*)(((char*)info.address) + (y * info.pitch) + (x * info.pixel_size));

        uint16_t char_data = ((uint16_t)ega_attributes << 8) | character;

        *target_address = char_data;

        x++;
    }
    return;
}



/**
 * @brief Write the string at the address, to the screen
 * 
 * @param x                 Starting X coordinate
 * @param y                 Starting Y coordinate
 * @param ega_attributes    EGA/VGA Attributes for the printed string
 * @param string_addr       Address of the null-terminated string to write
 */
void v_fb::ega_puts(uint32_t& x, uint32_t& y, uint8_t ega_attributes, const char* string) {
    uint32_t index = 0;
    uint32_t x_cnt = x;
    uint32_t y_cnt = y;

    while(1) {
        char target_char = *(string + index);

        // Identify special characters
        if (target_char == '\0') {
            x = x_cnt;
            y = y_cnt;
            break;
        } else if (target_char == '\n') {
            y_cnt++;
            x_cnt = x;
        } else if (target_char == '\t') {
            x_cnt += 4;
        } else if (target_char == '\b') {
            if (x_cnt < 1) {
                x_cnt = info.width;
                y--;
            }
            x_cnt--;
        } else {
            ega_putc(x_cnt, y_cnt, ega_attributes, target_char);
        }
        
        // Wrap around if it's hit the edge of the screen
        if (x_cnt > info.width) {
            x_cnt = x;
            y_cnt++;
        }

        // Wrap around if it's hit the bottom of the screen
        if (y_cnt > info.height) {
            y_cnt = y;
            ega_blank(ega_attributes);
        }

        index++;
    }

    x = x_cnt;
    y = y_cnt;
}

/**
 * @brief Blanks the screen with the color inputted
 * 
 * @param ega_attributes    EGA/VGA Attributes to blank the screen with
 */
void v_fb::ega_blank(uint8_t ega_attributes) {
    for (uint32_t y = 0; y <= info.height; y++) {
        for (uint32_t x = 0; x <= info.width; x++) {
            ega_putc(x, y, ega_attributes, ' ');
        }
    }
}

/* #endregion */

/* #region FRAMEBUFFER FUNCTIONS */

inline void* v_fb::get_target_address(uint32_t x, uint32_t y) {

    if (x > info.width || y > info.height) {
        raise_error(301, const_cast<char*>("draw_pixel"));
        return 0;
    }

    return (void*)((uint32_t)info.address + (y * info.pitch) + (x * info.pixel_size));
}

/**
 * @brief Places a pixel of a certain color at the specified position
 * 
 * @param x         X position
 * @param y         Y position
 * @param color     Color of pixel to be drawn
 */
inline void v_fb::draw_pixel(uint32_t x, uint32_t y, uint32_t color) {

    draw_pixel(get_target_address(x, y), color);
}

/**
 * @brief Places a pixel of a certain color at the specified address
 * 
 * @param target_address    Target address to draw the pixel
 * @param color             Color of pixel to be drawn
 */
void v_fb::draw_pixel(void* target_address, uint32_t color) {

    if (target_address > info.end) {
        raise_error(301, const_cast<char*>("draw_pixel"));
        return;
    }

    switch (info.depth) {
        case 8:
            *(uint8_t*) target_address = (uint8_t)(color & 0xff);
            break;

        case 32:
            // 32 bit color starts with an alpha value
            ((uint8_t*)target_address)[3] = (unsigned char) 0xff;
            /* fall through */

        case 24:
            ((uint8_t*)target_address)[2] = (unsigned char)((color & 0xff0000) >> 16);
            /* fall through */

        case 16:
            *(uint16_t*) target_address = (uint16_t)(color & 0xffff);
            break;

        default:
            for (uint8_t bits = 0; bits <= info.depth; bits += 8) {
                *(uint8_t*) target_address = (uint8_t)((color >> bits) & 0xff);
                target_address = (void*)((uintptr_t)target_address + 1);
            }
    }
}

/**
 * @brief Draws a line using Bresenham's algorithm (or a more direct handler 
 *        for directly horizontal or veritcal lines), with a specified color.
 * 
 * @param x0    Starting point's x-value
 * @param y0    Starting point's y-value
 * @param x1    Ending point's x-value
 * @param y1    Ending point's y-value
 * @param color Color for the drawn line
 */
void v_fb::draw_line(int x0, int y0, int x1, int y1, uint32_t color) {

    // Specific handler for vertical lines
    if ( x0 == x1) {
        if (y0 > y1) {
            int tempY = y1;
            y1 = y0;
            y0 = tempY;
        }

        void* target_address = get_target_address(x0, y0);
        for (y0 = y0; y0 <= y1; y0++ ) {
            draw_pixel(target_address, color);
            target_address = (void*)((uintptr_t)target_address + info.pitch);
        }
        return;
    }

    // Specific handler for horizontal lines
    if ( y0 == y1) {
        if (x0 > x1) {
            int tempX = x1;
            x1 = x0;
            x0 = tempX;
        }

        void* target_address = get_target_address(x0, y0);
        for (x0 = x0; x0 <= x1; x0++ ) {
            draw_pixel(target_address, color);
            target_address = (void*)((uintptr_t)target_address + info.pixel_size);
        }
        return;
    }

    // Implementation of Bresenham's Algorithm for diagonal lines

    int delta_x;
    int step_x;

    // Set intial values for delta_x and step_x
    if ( x1 > x0 ) {
        delta_x = x1 - x0;
        step_x = 1;
    } else {
        delta_x = x0 - x1;
        step_x = -1;
    }

    int delta_y_neg;
    int step_y;

    // Repeat for delta_y and step_y
    if ( y1 > y0 ) {
        delta_y_neg = -(y1 - y0);
        step_y = 1;
    } else {
        delta_y_neg = -(y0 - y1);
        step_y = -1;
    }

    int error = delta_x + delta_y_neg;
    int doubled_error;

    while (1) {
        draw_pixel(x0, y0, color);

        // If at end of line, break the while loop
        if (x0 == x1 && y0 == y1) { break; }

        doubled_error = error << 1;
        if (doubled_error >= delta_y_neg) {
            x0 += step_x;
            error += delta_y_neg;
        }

        if (doubled_error <= delta_x) {
            y0 += step_y;
            error += delta_x;
        }
    }

}

/**
 * @brief Draws the outline of a rectangle from (x_0, y_0) to
 *        (x_1, y_1), with a border thickness pixels thick, 
 *        on the outside of the rectangle, unless the inside_border
 *        flag is set
 * 
 * @param x_0             X value of first point
 * @param y_0             Y value of first point
 * @param x_1             X value of second point
 * @param y_1             Y value of second point
 * @param color           Color of outline
 * @param thickness       Thickness of border
 * @param inside_border   Flag to set border to be on the inside, otherwise 
 *                        it will be on the outside
 */
void v_fb::draw_rect(uint32_t x_0, uint32_t y_0, uint32_t x_1, uint32_t y_1, uint32_t color, uint16_t thickness, bool inside_border) {

    if (thickness == 0) {
        draw_line(x_0, y_0, x_1, y_0, color);
        draw_line(x_0, y_0, x_0, y_1, color);
        draw_line(x_1, y_0, x_1, y_1, color);
        draw_line(x_0, y_1, x_1, y_1, color);
        return;
    }

    // Standard modifier for drawing the borders
    uint16_t border_mod;
    int8_t border_it;

    // Set border modifier negative if its on the outside
    if (!inside_border) {
        border_mod = -thickness;
        border_it = 1;
    } else {
        border_mod = thickness;
        border_it = -1;
    }

    uint32_t y_i;
    uint32_t x_i;

    // Top edge
    for (y_i = (y_0 + border_mod); y_i != y_0; y_i += border_it) {
        for (x_i = x_0; x_i <= x_1; x_i++) {
            draw_pixel(x_i, y_i, color);
        }
    }

    // Bottom edge
    for (y_i = (y_1 - border_mod); y_i != y_1; y_i -= border_it) {
        for (x_i = x_0; x_i <= x_1; x_i++) {
            draw_pixel(x_i, y_i, color);
        }
    }

    // Left edge
    for (x_i = (x_0 + border_mod); x_i != x_0; x_i += border_it) {
        for (y_i = y_0; y_i <= y_1; y_i++) {
            draw_pixel(x_i, y_i, color);
        }
    }

    // Right edge
    for (x_i = (x_1 - border_mod); x_i != x_1; x_i -= border_it) {
        for (y_i = y_0; y_i <= y_1; y_i++) {
            draw_pixel(x_i, y_i, color);
        }
    }
}

/**
 * @brief Draws a filled in rectangle from (x0, y0) to (x1, y1)
 * 
 * @param x0              X value of first point
 * @param y0              Y value of first point
 * @param x1              X value of second point
 * @param y1              Y value of second point
 * @param color            Color of rectangle to draw
 */
void v_fb::draw_rect_fill(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color) {

    // Basic filling algorithm
    void* target_address;
    void* next_row = get_target_address(x0, y0);
    for (; y0 <= y1; y0++) {
        target_address = next_row;
        for (unsigned int xi = x0; xi <= x1; xi++) {
            draw_pixel(target_address, color);
            target_address = (void*)((uintptr_t)target_address + info.pixel_size);
        }
        next_row = (void*)((uintptr_t)next_row + info.pitch);
    }
}

/**
 * @brief Draws a character to the framebuffer
 * 
 * @param x             X value for top-left pixel of character
 * @param y             Y value for top-left pixel of character
 * @param target_char   Character to draw
 * @param fg_color      Foreground character color
 * @param bg_color      Background character color
 */
void v_fb::fb_putc(uint32_t& x, uint32_t& y, const char target_char, uint32_t fg_color, uint32_t bg_color) {

    if (target_char == 0 || target_char > font->char_count) {
        return;
    }

    int char_width = font->char_width;
    int char_height = font->char_height;

    // Switch for special characters
    switch (target_char) {

        case '\n':
            x = 0;
            y += char_height;
            return;

        case '\t':
            x += (char_width * 4);
            return;

        case '\b':
            if (x > (unsigned int)char_width) {
                x -= char_width;
                fb_putc(x, y, ' ', fg_color, bg_color);
                x -= char_width;
            }
            return;

        default:
            {
            // Get data for this char
            const unsigned int* char_data = font->get_char(target_char);

            // Calculate starting address and offset to next line
            void* target_address = get_target_address(x, y);
            size_t line_offset = info.pitch - (char_width * info.pixel_size);

            for (int yIt = 0; yIt < char_height; yIt++) {

                unsigned int line_data = char_data[yIt];
                for (int i = char_width; i > 0;) {
                        i--;
                        if (line_data & (1 << i))
                            draw_pixel(target_address, fg_color);
                        else
                            draw_pixel(target_address, bg_color);

                        target_address = (void*)((uintptr_t)target_address + info.pixel_size);
                }
                target_address = (void*)((uintptr_t)target_address + line_offset);
            }
            
            x += char_width;
            }
            break;
    }

   // If reached edge of screen, wrap around
    if (x >= info.width) {
        x = 0;
        y += char_height;
    }

    // If reached end of screen, wrap around vertically
    if (y >= info.height) {
        y = 0;
        fb_blank(bg_color);
    }
}

/**
 * @brief Draws a null-terminated string to the framebuffer, and leaves 
 *        the position of the next character in x and y
 * 
 * @param x             X-value for top-left pixel of first char in the string
 * @param y             Y value for top-left pixel of first char in the string
 * @param string        Null-terminated string to be drawn
 * @param fg_color      Foreground character color
 * @param bg_color      Background character color
 */
void v_fb::fb_puts(uint32_t& x, uint32_t& y, const char* string, uint32_t fg_color, uint32_t bg_color) {

    uint32_t char_it = 0;

    // Loop through string until null termination
    while(1) {
        char target_char = string[char_it];

        // Check for null termination
        if (target_char == '\0') {
            return;
        }
        
        fb_putc(x, y, target_char, fg_color, bg_color);

        char_it++;
    }

}

/**
 * @brief Blank the entire framebuffer with a certain color. Not guaranteed
 *        to perform correctly with non-greyscale colors in certain bit depths,
 *        where draw_rect_fill over the entire screen would be a better choice.
 * 
 * @param color     Color to fill screen with
 */
void v_fb::fb_blank(uint32_t color) {

    // Turn the color into a fill pattern 
    unsigned int fill = 0;
    for (size_t bits = 0; bits < sizeof(unsigned int); bits += info.depth) {
        fill |= (color << bits);
    }

    // Fill the framebuffer portion of memory
    fill_mem(info.address, info.size, fill);
}

/**
 * @brief Draws a series of pixels in a bit format, where each bit is one pixel,
 *        and a 1 = fg, and 0 = bg fill. This performs this in a manner more
 *        efficient than individually filling each pixel.
 * 
 * @param x         X-value to start at
 * @param y         Y-value to print at
 * @param bmp       Bit string/map of data to fill pixels with
 * @param length    Length of bmp / string to be printed
 * @param fg        Foreground color / 1 in string
 * @param bg        Background color / 0 in string
 */
void v_fb::fb_place_bmp(uint32_t x, uint32_t y, unsigned int bmp, uint32_t length, uint32_t fg, uint32_t bg) {

    // Get target address
    void* target_address = get_target_address(x, y);

    for (unsigned int index = 0; index < length; index++) {
        if (bmp & (1 << index))
            draw_pixel(target_address, fg);
        else
            draw_pixel(target_address, bg);
        
        target_address = (void*)((uintptr_t)target_address + info.pixel_size);
    }
}

/* #endregion */

void v_fb::show(uint32_t x, uint32_t y) {

    unsigned char* source = (unsigned char*)info.address;
    unsigned char* target = (unsigned char*)fb.address + (fb.pitch * y) + (fb.pixel_size * x);

    if (info.width < fb.width) {

        for (unsigned int row = 0; row < info.height; row++) {

            memcpy(target, source, info.pitch);
            target += fb.pitch;
            source += info.pitch;
        }

    } else {

        memcpy(target, source, info.size);
    }
}

void v_fb::blank(uint32_t color) {
    fb_blank(color);
}

void v_fb::draw_c(uint32_t& x, uint32_t& y, char target_char, 
    uint32_t fg, uint32_t bg, uint32_t ega) {

    if (info.direct_color)
        fb_putc(x, y, target_char, fg, bg);
    else
        ega_putc(x, y, ega, target_char);
}

void v_fb::draw_s(uint32_t& x, uint32_t& y, const char* string, 
    uint32_t fg, uint32_t bg, uint32_t ega) {

    if (info.direct_color)
        fb_puts(x, y, string, fg, bg);
    else
        ega_puts(x, y, ega, string);
}


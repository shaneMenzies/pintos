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
void ega_putc(uint32_t x, uint32_t y, uint8_t ega_attributes, char character) {
    if(fb_initialized && fb.ega_text) {

        uint16_t* target_address = (uint16_t*)(((char*)fb.address) + (y * fb.pitch) + (x * fb.pixel_size));

        uint16_t char_data = ((uint16_t)ega_attributes << 8) | character;

        *target_address = char_data;
    }
    return;
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

/**
 * @brief Write the string at the address, to the screen
 * 
 * @param x                 Starting X coordinate
 * @param y                 Starting Y coordinate
 * @param ega_attributes    EGA/VGA Attributes for the printed string
 * @param string_addr       Address of the null-terminated string to write
 */
void ega_puts(uint32_t x, uint32_t y, uint8_t ega_attributes, char string[]) {
    uint32_t index = 0;
    uint32_t x_cnt = x;
    uint32_t y_cnt = y;

    while(1) {
        char target_char = *(string + index);

        // Identify special characters
        if (target_char == '\0') {
            return;
        } else if (target_char == '\n') {
            y_cnt++;
            x_cnt = x;
        } else if (target_char == '\t') {
            x_cnt += 4;
        } else if (target_char == '\b') {
            if (x_cnt < 1) {
                x_cnt = fb.width;
                y--;
            }
            x_cnt--;
        } else {
            ega_putc(x_cnt, y_cnt, ega_attributes, target_char);
            x_cnt++;
        }
        
        // Wrap around if it's hit the edge of the screen
        if (x_cnt > fb.width) {
            x_cnt = x;
            y_cnt++;
        }

        // Wrap around if it's hit the bottom of the screen
        if (y_cnt > fb.height) {
            y_cnt = y;
            ega_blank(ega_attributes);
        }

        index++;
    }
}

/**
 * @brief Blanks the screen with the color inputted
 * 
 * @param ega_attributes    EGA/VGA Attributes to blank the screen with
 */
void ega_blank(uint8_t ega_attributes) {
    for (uint32_t y = 0; y <= fb.height; y++) {
        for (uint32_t x = 0; x <= fb.width; x++) {
            ega_putc(x, y, ega_attributes, ' ');
        }
    }
}

/* #endregion */

/* #region FRAMEBUFFER FUNCTIONS */

inline void* get_pixel_address(uint32_t x, uint32_t y) {

    if (x > fb.width || y > fb.height) {
        raise_error(301, const_cast<char*>("draw_pixel"));
        return 0;
    }

    return (void*)((uint32_t)fb.address + (y * fb.pitch) + (x * fb.pixel_size));
}

/**
 * @brief Places a pixel of a certain color at the specified position
 * 
 * @param x         X position
 * @param y         Y position
 * @param color     Color of pixel to be drawn
 */
inline void draw_pixel(uint32_t x, uint32_t y, uint32_t color) {

    draw_pixel(get_pixel_address(x, y), color);
}

/**
 * @brief Places a pixel of a certain color at the specified address
 * 
 * @param target_address    Target address to draw the pixel
 * @param color             Color of pixel to be drawn
 */
void draw_pixel(void* target_address, uint32_t color) {

    if (target_address > fb.end) {
        raise_error(301, const_cast<char*>("draw_pixel"));
        return;
    }

    switch (fb.depth) {
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
            for (uint8_t bits = 0; bits <= fb.depth; bits += 8) {
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
void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {

    // Specific handler for vertical lines
    if ( x0 == x1) {
        if (y0 > y1) {
            int tempY = y1;
            y1 = y0;
            y0 = tempY;
        }

        void* target_address = get_pixel_address(x0, y0);
        for (y0 = y0; y0 <= y1; y0++ ) {
            draw_pixel(target_address, color);
            target_address = (void*)((uintptr_t)target_address + fb.pitch);
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

        void* target_address = get_pixel_address(x0, y0);
        for (x0 = x0; x0 <= x1; x0++ ) {
            draw_pixel(target_address, color);
            target_address = (void*)((uintptr_t)target_address + fb.pixel_size);
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
void draw_rect(uint32_t x_0, uint32_t y_0, uint32_t x_1, uint32_t y_1, uint32_t color, uint16_t thickness, bool inside_border) {

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
void draw_rect_fill(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color) {

    // Basic filling algorithm
    void* target_address = get_pixel_address(x0, y0);
    for (y0 = y0; y0 <= y1; y0++) {
        for (unsigned int xi = x0; xi <= x1; xi++) {
            draw_pixel(target_address, color);
            target_address = (void*)((uintptr_t)target_address + fb.pixel_size);
        }
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
void fb_putc(uint32_t x, uint32_t y, char target_char, uint32_t fg_color, uint32_t bg_color) {

    if (target_char > glyph_char_count) {
        target_char = 0;
    }

    unsigned char uchar = (unsigned char) target_char;

    int char_width = glyph_width;
    int char_height = glyph_height;

    // Calculate starting address and offset to next line
    void* target_address = get_pixel_address(x, y);
    size_t line_offset = fb.pitch - (char_width * fb.pixel_size);

   for ( int yIt = 0; yIt < char_height; yIt++) {

       unsigned int char_data = glyph_bitmap[uchar][yIt];
       for (int i = char_width; i > 0;) {
            i--;
            if (char_data & (1 << i))
                draw_pixel(target_address, fg_color);
            else
                draw_pixel(target_address, bg_color);

            target_address = (void*)((uintptr_t)target_address + fb.pixel_size);
       }
       target_address = (void*)((uintptr_t)target_address + line_offset);
   } 
}

/**
 * @brief Draws a null-terminated string to the framebuffer
 * 
 * @param x             X-value for top-left pixel of first char in the string
 * @param y             Y value for top-left pixel of first char in the string
 * @param string        Null-terminated string to be drawn
 * @param fg_color      Foreground character color
 * @param bg_color      Background character color
 */
void fb_puts(uint32_t x, uint32_t y, char string[], uint32_t fg_color, uint32_t bg_color) {

    uint8_t char_width = glyph_width;
    uint8_t char_height = glyph_height;

    uint32_t x_i = x;
    uint32_t y_i = y;

    uint32_t char_it = 0;

    // Loop through string until null termination
    while(1) {
        char target_char = string[char_it];

        // Switch for special characters
        switch (target_char) {

            case '\0':
                return;

            case '\n':
                x_i = x;
                y_i += char_height;
                break;

            case '\t':
                x_i += (char_width * 4);
                break;

            case '\b':
                if (x_i == 0) {
                    x_i = fb.width;
                    y_i -= char_height;
                }
                x_i -= char_width;
                break;

            default:
                fb_putc(x_i, y_i, target_char, fg_color, bg_color);
                x_i += char_width;
        }

        // If reached edge of screen, wrap around
        if (x_i >= fb.width) {
            x_i = x;
            y_i += char_height;
        }

        // If reached end of screen, wrap around vertically
        if (y_i >= fb.height) {
            y_i = y;
            fb_blank(bg_color);
        }

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
void fb_blank(uint32_t color) {

    // Turn the color into a fill pattern 
    unsigned int fill = 0;
    for (size_t bits = 0; bits < sizeof(unsigned int); bits += fb.depth) {
        fill |= (color << bits);
    }

    // Fill the framebuffer portion of memory
    fill_mem(fb.address, fb.size, fill);
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
void fb_place_bmp(uint32_t x, uint32_t y, unsigned int bmp, uint32_t length, uint32_t fg, uint32_t bg) {

    // Get target address
    void* target_address = get_pixel_address(x, y);

    for (unsigned int index = 0; index < length; index++) {
        if (bmp & (1 << index))
            draw_pixel(target_address, fg);
        else
            draw_pixel(target_address, bg);
        
        target_address = (void*)((uintptr_t)target_address + fb.pixel_size);
    }
}

/* #endregion */


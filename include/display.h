#ifndef DISPLAY_H
#define DISPLAY_H

#include "multiboot.h"

#include <stdbool.h>
#include <stdint.h>

extern bool fb_initialized;

struct fb_info {

    void* address;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t pixel_size;     // Actual Bytes per pixel
    uint8_t depth;          // Color depth in bits per pixel

    bool palette_color;
    uint32_t palette_addr;
    uint16_t palette_num;

    bool direct_color;
    uint8_t red_pos;
    uint8_t red_mask;
    uint8_t green_pos;
    uint8_t green_mask;
    uint8_t blue_pos;
    uint8_t blue_mask;

    bool ega_text;
};

extern struct fb_info fb;

typedef struct uint24_t {
    unsigned char byte[3];
} uint24_t;

void framebuffer_init(struct mb_info* mb_addr);

void ega_putc(uint32_t x, uint32_t y, uint8_t ega_attributes, char character);

uint8_t ega_attributes(uint8_t fg_color, uint8_t bg_color);

void ega_puts(uint32_t x, uint32_t y, uint8_t ega_attributes, char string[]);

void ega_blank(uint8_t ega_attributes);

void draw_pixel(uint32_t x, uint32_t y, uint32_t color);

void draw_line(int x0, int y0, int x1, int y1, uint32_t color);

void draw_rect(uint32_t x_0, uint32_t y_0, uint32_t x_1, uint32_t y_1, 
               uint32_t color, uint16_t thickness, bool inside_border);

void draw_rect_fill(uint32_t x_0, uint32_t y_0, uint32_t x_1, uint32_t y_1, 
                    uint32_t color);

void fb_putc(uint32_t x, uint32_t y, char target_char, uint32_t fg_color, 
             uint32_t bg_color); 

void fb_puts(uint32_t x, uint32_t y, char string[], uint32_t fg_color, 
             uint32_t bg_color);

void fb_blank(uint32_t color);

#endif
#ifndef DISPLAY_H
#define DISPLAY_H

#include "fonts/ibm_pc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

class visual_terminal;
struct multiboot_boot_info;

extern bool fb_initialized;

struct fb_info {

    void*    address;
    void*    end;
    size_t   size;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t  pixel_size; // Actual Bytes per pixel
    uint8_t  depth;      // Color depth in bits per pixel

    bool    direct_color;
    uint8_t red_shift;
    uint8_t red_mask;
    uint8_t green_shift;
    uint8_t green_mask;
    uint8_t blue_shift;
    uint8_t blue_mask;

    bool ega_text;

    fb_info() {}
    fb_info(const fb_info& other) {

        address      = other.address;
        end          = other.end;
        size         = other.size;
        pitch        = other.pitch;
        width        = other.width;
        height       = other.height;
        pixel_size   = other.pixel_size;
        depth        = other.depth;
        direct_color = other.direct_color;
        red_shift    = other.red_shift;
        red_mask     = other.red_mask;
        green_shift  = other.green_shift;
        green_mask   = other.green_mask;
        blue_shift   = other.blue_shift;
        blue_mask    = other.blue_mask;
        ega_text     = other.ega_text;
    }

    // Copy operator
    fb_info& operator=(const fb_info& other) {

        if (this == &other) { return *this; }

        address      = other.address;
        end          = other.end;
        size         = other.size;
        pitch        = other.pitch;
        width        = other.width;
        height       = other.height;
        pixel_size   = other.pixel_size;
        depth        = other.depth;
        direct_color = other.direct_color;
        red_shift    = other.red_shift;
        red_mask     = other.red_mask;
        green_shift  = other.green_shift;
        green_mask   = other.green_mask;
        blue_shift   = other.blue_shift;
        blue_mask    = other.blue_mask;
        ega_text     = other.ega_text;

        return *this;
    }
};

class v_fb {

    friend class visual_terminal;

  private:
    const Font*  font = &ibm_pc_8x16;
    inline void* get_target_address(uint32_t x, uint32_t y);

    // Text-mode functions
    void ega_putc(uint32_t& x, uint32_t& y, uint8_t ega_attributes,
                  const char character);
    void ega_puts(uint32_t& x, uint32_t& y, uint8_t ega_attributes,
                  const char* string);
    void ega_blank(uint8_t ega_attributes);

    // Pixel buffer functions
    void fb_putc(uint32_t& x, uint32_t& y, const char target_char,
                 uint32_t fg_color, uint32_t bg_color);
    void fb_puts(uint32_t& x, uint32_t& y, const char* string,
                 uint32_t fg_color, uint32_t bg_color);
    void fb_blank(uint8_t color);
    void fb_place_bmp(uint32_t x, uint32_t y, unsigned int bmp, uint32_t length,
                      uint32_t fg, uint32_t bg);

  public:
    fb_info  info;
    uint16_t char_width;
    uint16_t char_height;
    size_t   char_pitch;

    v_fb();
    v_fb(uint32_t width, uint32_t height);
    ~v_fb();

    void        set_font(const Font* new_font);
    const Font* get_font() { return font; }

    void show(uint32_t x = 0, uint32_t y = 0);
    void blank(uint8_t color);

    // Printing characters and strings
    void draw_c(uint32_t& x, uint32_t& y, const char target_char, uint32_t fg,
                uint32_t bg, uint32_t ega);
    void draw_s(uint32_t& x, uint32_t& y, const char* string, uint32_t fg,
                uint32_t bg, uint32_t ega);

    // Pixel buffer only functions
    inline void draw_pixel(uint32_t x, uint32_t y, uint32_t color);
    void        draw_pixel(void* target_address, uint32_t color);
    void        draw_line(int x0, int y0, int x1, int y1, uint32_t color);
    void draw_rect(uint32_t x_0, uint32_t y_0, uint32_t x_1, uint32_t y_1,
                   uint32_t color, uint16_t thickness, bool inside_border);
    void draw_rect_fill(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1,
                        uint32_t color);
};

typedef struct uint24_t {
    unsigned char byte[3];
} uint24_t;

void framebuffer_init(multiboot_boot_info* mb_info);

inline uint8_t ega_attributes(uint8_t fg_color, uint8_t bg_color);

#endif

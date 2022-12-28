#ifndef TERMINAL_DEF_H
#define TERMINAL_DEF_H

#include "display/display.h"
#include "io/keyboard.h"
#include "libk/streambuf.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

class terminal : public std_k::streambuf {

    friend class visual_terminal;

  private:
    char* start;
    char* next;
    char* end;

    keyboard::kb_handler handler;

  protected:
    // Streambuf overrides
    virtual int sync() override;

    virtual size_t seekoff(int off, std_k::ios_base::seekdir dir,
                           std_k::ios_base::openmode side) override;
    virtual size_t seekpos(size_t pos, std_k::ios_base::openmode side) override;

    virtual int    overflow(char_type value) override;
    virtual size_t xsputn(const char_type* source, size_t count) override;

  public:
    terminal(size_t text_size = 65536);
    terminal(keyboard::kb_handler handler, size_t text_size = 65536);
    ~terminal();

    void                set_handler(keyboard::kb_handler new_handler);
    virtual inline void send_key(char character);

    virtual void write_c(const char character);
    virtual void write_s(const char* string);
    virtual void write_n(const char* string, size_t length);

    void tprintf(const char* format, ...);
    void vtprintf(const char* format, va_list args);

    virtual void tclear();
};

class visual_terminal : public terminal {

  private:
    v_fb     fb;
    v_fb     cursor;
    uint32_t x_pos         = 0;
    uint32_t y_pos         = 0;
    bool     cursor_active = true;

  public:
    uint32_t     default_fg, default_bg;
    uint8_t      default_ega;
    const double target_fill;
    unsigned int target_height;
    unsigned int scroll_shift;

    visual_terminal(size_t text_size = 65536, uint32_t fg = 0xffffff,
                    uint32_t bg = 0, uint8_t ega = 0x0f,
                    double target_fill = 0.9f);

    inline void send_key(char character) override {
        write_c(character);
        handler.run_action(character);
    }
    void write_c(const char character) override;
    void         write_s(const char* string) override;
    virtual void write_n(const char* string, size_t length) override;
    void         draw_cursor(int state = -1);

    void tclear() override;

    inline void update() {
        fb.show();
        draw_cursor(1);
    }
};

#endif

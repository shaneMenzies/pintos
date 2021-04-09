#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "terminal.h"
#include "libk.h"
#include "io.h"

namespace keyboard {

    enum key : unsigned char {
        LEFT_SHIFT = 0x2a,
        RIGHT_SHIFT = 0x36,
        LEFT_ALT = 0x38,
        LEFT_CTRL = 0x1d,
        ENTER = 0x1c,
        ESC = 0x01,
        CAPS_LOCK = 0x3a,
        ACK = 0xfa,
    };

    enum e0_key : unsigned char {
        RIGHT_ALT = 0x38,
        RIGHT_CTRL = 0x1d,
        LEFT_SYSTEM = 0x5b,
        RIGHT_SYSTEM = 0x5c,
        INSERT = 0x52,
        HOME = 0x47,
        PAGE_UP = 0x49,
        PAGE_DOWN = 0x51,
        DELETE = 0x53,
        END = 0x4f,
        UP = 0x48,
        LEFT = 0x4b,
        RIGHT = 0x4d,
        DOWN = 0x50,

        KP_FW_SLASH = 0x35,
        KP_ENTER = 0x1c,
    };

    enum locks : unsigned char {
        SCROLL,
        NUM,
        CAPS,
        E0,
    };

    extern bool made[0x100];
    extern bool e0_made[0x100];
    extern bool lock[8];

    bool shift_active();
    bool ctrl_active();
    bool alt_active();

    void key_make(unsigned char key);
    void key_break(unsigned char key);

    void update_lights();
    void wait_ack();
}

#endif
/**
 * @file keyboard.cpp
 * @author Shane Menzies
 * @brief Keyboard functions
 * @date 03/16/21
 * 
 * 
 */

#pragma GCC diagnostic ignored "-Wchar-subscripts"

#include "keyboard.h"

#include "terminal.h"
#include "libk.h"
#include "io.h"

namespace keyboard {

    /**
     * @brief Translation of keyboard scan code set 1
     * 
     */
    const char code_translation[0x100] = {

        // MAKE CODES (Key pressed)

        0,  // 0x00 - Keyboard error/overflow
        0,  // 0x01 - Escape
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
        '0',
        '-',
        '=',
        '\b',
        '\t',
        'q',
        'w',
        'e',
        'r',
        't',
        'y',
        'u',
        'i',
        'o',
        'p',
        '[',
        ']', // 0x1b    
        '\n',  // 0x1c - Enter
        0,  // 0x1d
        'a',  // 0x1e
        's',  // 0x1f
        'd',  // 0x20
        'f',  // 0x21
        'g',  // 0x22
        'h',  // 0x23
        'j',  // 0x24
        'k',  // 0x25
        'l',  // 0x26
        ';',  // 0x27
        '\'',  // 0x28
        0,  // 0x29
        0,  // 0x2a - Left Shift
        '\\',// 0x2b
        'z',  // 0x2c
        'x',  // 0x2d
        'c',  // 0x2e
        'v',  // 0x2f
        'b',  // 0x30
        'n',  // 0x31
        'm',  // 0x32
        ',',  // 0x33
        '.',  // 0x34
        '/',  // 0x35
        0,  // 0x36 - Right Shift
        '*',  // 0x37 - Keypad *
        0,  // 0x38 - Left Alt
        ' ',  // 0x39
        0,  // 0x3a - Caps Lock
        0,  // 0x3b - F1
        0,  // 0x3c - F2
        0,  // 0x3d - F3
        0,  // 0x3e - F4
        0,  // 0x3f - F5
        0,  // 0x40 - F6
        0,  // 0x41 - F7
        0,  // 0x42 - F8
        0,  // 0x43 - F9
        0,  // 0x44 - F10
        0,  // 0x45 - Num Lock
        0,  // 0x46 - Scroll Lock
        '7',  // 0x47 - Keypad 7 / Home
        '8',  // 0x48 - Keypad 8 / Up
        '9',  // 0x49 - Keypad 9 / Page Up
        '-',  // 0x4a - Keypad - 
        '4',  // 0x4b - Keypad 4 / Left
        '5',  // 0x4c - Keypad 5
        '6',  // 0x4d - Keypad 6 / Right
        '+',  // 0x4e - Keypad +
        '1',  // 0x4f - Keypad 1 / End
        '2',  // 0x50 - Keypad 2 / Down
        '3',  // 0x51 - Keypad 3 / Page Down
        '0',  // 0x52 - Keypad 0 / Insert
        '.',  // 0x53 - Keypad . / Delete
        0,  // 0x54 - Alt + SysReq
        0,  // 0x55
        0,  // 0x56
        0,  // 0x57 - F11
        0,  // 0x58 - F12
        0,  // 0x59
        0,  // 0x5a
        0,  // 0x5b
        0,  // 0x5c
        0,  // 0x5d
        0,  // 0x5e
        0,  // 0x5f

        // Filler before Break codes
        0,  // 0x60
        0,  // 0x61
        0,  // 0x62
        0,  // 0x63
        0,  // 0x64
        0,  // 0x65
        0,  // 0x66
        0,  // 0x67
        0,  // 0x68
        0,  // 0x69
        0,  // 0x6a
        0,  // 0x6b
        0,  // 0x6c
        0,  // 0x6d
        0,  // 0x6e
        0,  // 0x6f
        0,  // 0x70
        0,  // 0x71
        0,  // 0x72
        0,  // 0x73
        0,  // 0x74
        0,  // 0x75
        0,  // 0x76
        0,  // 0x77
        0,  // 0x78
        0,  // 0x79
        0,  // 0x7a
        0,  // 0x7b
        0,  // 0x7c
        0,  // 0x7d
        0,  // 0x7e
        0,  // 0x7f

        // BREAK CODES (Key released)

        0,  // 0x80 - non-US-1
        0,  // 0x81 - Escape
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
        '0',
        '-',
        '=',
        '\b',
        '\t',
        'q',
        'w',
        'e',
        'r',
        't',
        'y',
        'u',
        'i',
        'o',
        'p',
        '[',
        ']', // 0x9b    
        '\n',  // 0x9c - Enter
        0,  // 0x9d
        'a',  // 0x9e
        's',  // 0x9f
        'd',  // 0xa0
        'f',  // 0xa1
        'g',  // 0xa2
        'h',  // 0xa3
        'j',  // 0xa4
        'k',  // 0xa5
        'l',  // 0xa6
        ';',  // 0xa7
        '\'',  // 0xa8
        0,  // 0xa9
        0,  // 0xaa - Left Shift
        '\\',// 0xab
        'z',  // 0xac
        'x',  // 0xad
        'c',  // 0xae
        'v',  // 0xaf
        'b',  // 0xb0
        'n',  // 0xb1
        'm',  // 0xb2
        ',',  // 0xb3
        '.',  // 0xb4
        '/',  // 0xb5
        0,  // 0xb6 - Right Shift
        '*',  // 0xb7 - Keypad *
        0,  // 0xb8 - Left Alt
        ' ',  // 0xb9
        0,  // 0xba - Caps Lock
        0,  // 0xbb - F1
        0,  // 0xbc - F2
        0,  // 0xbd - F3
        0,  // 0xbe - F4
        0,  // 0xbf - F5
        0,  // 0xc0 - F6
        0,  // 0xc1 - F7
        0,  // 0xc2 - F8
        0,  // 0xc3 - F9
        0,  // 0xc4 - F10
        0,  // 0xc5 - Num Lock
        0,  // 0xc6 - Scroll Lock
        '7',  // 0xc7 - Keypad 7 / Home
        '8',  // 0xc8 - Keypad 8 / Up
        '9',  // 0xc9 - Keypad 9 / Page Up
        '-',  // 0xca - Keypad - 
        '4',  // 0xcb - Keypad 4 / Left
        '5',  // 0xcc - Keypad 5
        '6',  // 0xcd - Keypad 6 / Right
        '+',  // 0xce - Keypad +
        '1',  // 0xcf - Keypad 1 / End
        '2',  // 0xd0 - Keypad 2 / Down
        '3',  // 0xd1 - Keypad 3 / Page Down
        '0',  // 0xd2 - Keypad 0 / Insert
        '.',  // 0xd3 - Keypad . / Delete
        0,  // 0xd4 - Alt + SysReq (????)
        0,  // 0xd5
        0,  // 0xd6
        0,  // 0xd7 - F11
        0,  // 0xd8 - F12

        0,  // 0xd9
        0,  // 0xda
        0,  // 0xdb
        0,  // 0xdc
        0,  // 0xdd
        0,  // 0xde
        0,  // 0xdf
        0,  // 0xe0 - Two byte scan code
        0,  // 0xe1
        0,  // 0xe2
        0,  // 0xe3
        0,  // 0xe4
        0,  // 0xe5
        0,  // 0xe6
        0,  // 0xe7
        0,  // 0xe8
        0,  // 0xe9
        0,  // 0xea
        0,  // 0xeb
        0,  // 0xec
        0,  // 0xed
        0,  // 0xee
        0,  // 0xef
        0,  // 0xf0
        0,  // 0xf1
        0,  // 0xf2
        0,  // 0xf3
        0,  // 0xf4
        0,  // 0xf5
        0,  // 0xf6
        0,  // 0xf7
        0,  // 0xf8
        0,  // 0xf9
        0,  // 0xfa - Command Acknowledged
        0,  // 0xfb 
        0,  // 0xfc
        0,  // 0xfd
        0,  // 0xfe - Repeat last command
        0,  // 0xff - Keyboard error/buffer overrun
    };

    const char shift_translation[0x100] = {

        // MAKE CODES (Key pressed)

        0,  // 0x00 - non-US-1
        0,  // 0x01 - Escape
        '!',
        '@',
        '#',
        '$',
        '%',
        '^',
        '&',
        '*',
        '(',
        ')',
        '_',
        '+',
        '\b',
        '\t',
        'Q',
        'W',
        'E',
        'R',
        'T',
        'Y',
        'U',
        'I',
        'O',
        'P',
        '{',
        '}', // 0x1b    
        '\n',  // 0x1c - Enter
        0,  // 0x1d
        'A',  // 0x1e
        'S',  // 0x1f
        'D',  // 0x20
        'F',  // 0x21
        'G',  // 0x22
        'H',  // 0x23
        'J',  // 0x24
        'K',  // 0x25
        'L',  // 0x26
        ':',  // 0x27
        '\"',  // 0x28
        0,  // 0x29
        0,  // 0x2a - Left Shift
        '|',// 0x2b
        'Z',  // 0x2c
        'X',  // 0x2d
        'C',  // 0x2e
        'V',  // 0x2f
        'B',  // 0x30
        'N',  // 0x31
        'M',  // 0x32
        '<',  // 0x33
        '>',  // 0x34
        '?',  // 0x35
        0,  // 0x36 - Right Shift
        '*',  // 0x37 - Keypad *
        0,  // 0x38 - Left Alt
        ' ',  // 0x39
        0,  // 0x3a - Caps Lock
        0,  // 0x3b - F1
        0,  // 0x3c - F2
        0,  // 0x3d - F3
        0,  // 0x3e - F4
        0,  // 0x3f - F5
        0,  // 0x40 - F6
        0,  // 0x41 - F7
        0,  // 0x42 - F8
        0,  // 0x43 - F9
        0,  // 0x44 - F10
        0,  // 0x45 - Num Lock
        0,  // 0x46 - Scroll Lock
        '7',  // 0x47 - Keypad 7 / Home
        '8',  // 0x48 - Keypad 8 / Up
        '9',  // 0x49 - Keypad 9 / Page Up
        '-',  // 0x4a - Keypad - 
        '4',  // 0x4b - Keypad 4 / Left
        '5',  // 0x4c - Keypad 5
        '6',  // 0x4d - Keypad 6 / Right
        '+',  // 0x4e - Keypad +
        '1',  // 0x4f - Keypad 1 / End
        '2',  // 0x50 - Keypad 2 / Down
        '3',  // 0x51 - Keypad 3 / Page Down
        '0',  // 0x52 - Keypad 0 / Insert
        '.',  // 0x53 - Keypad . / Delete
        0,  // 0x54 - Alt + SysReq
        0,  // 0x55
        0,  // 0x56
        0,  // 0x57 - F11
        0,  // 0x58 - F12
        0,  // 0x59
        0,  // 0x5a
        0,  // 0x5b
        0,  // 0x5c
        0,  // 0x5d
        0,  // 0x5e
        0,  // 0x5f

        // Filler before Break codes
        0,  // 0x60
        0,  // 0x61
        0,  // 0x62
        0,  // 0x63
        0,  // 0x64
        0,  // 0x65
        0,  // 0x66
        0,  // 0x67
        0,  // 0x68
        0,  // 0x69
        0,  // 0x6a
        0,  // 0x6b
        0,  // 0x6c
        0,  // 0x6d
        0,  // 0x6e
        0,  // 0x6f
        0,  // 0x70
        0,  // 0x71
        0,  // 0x72
        0,  // 0x73
        0,  // 0x74
        0,  // 0x75
        0,  // 0x76
        0,  // 0x77
        0,  // 0x78
        0,  // 0x79
        0,  // 0x7a
        0,  // 0x7b
        0,  // 0x7c
        0,  // 0x7d
        0,  // 0x7e
        0,  // 0x7f

        // BREAK CODES (Key released)

        0,  // 0x80 - non-US-1
        0,  // 0x81 - Escape
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
        '0',
        '-',
        '=',
        '\b',
        '\t',
        'q',
        'w',
        'e',
        'r',
        't',
        'y',
        'u',
        'i',
        'o',
        'p',
        '[',
        ']', // 0x9b    
        '\n',  // 0x9c - Enter
        0,  // 0x9d
        'a',  // 0x9e
        's',  // 0x9f
        'd',  // 0xa0
        'f',  // 0xa1
        'g',  // 0xa2
        'h',  // 0xa3
        'j',  // 0xa4
        'k',  // 0xa5
        'l',  // 0xa6
        ';',  // 0xa7
        '\'',  // 0xa8
        0,  // 0xa9
        0,  // 0xaa - Left Shift
        '\\',// 0xab
        'z',  // 0xac
        'x',  // 0xad
        'c',  // 0xae
        'v',  // 0xaf
        'b',  // 0xb0
        'n',  // 0xb1
        'm',  // 0xb2
        ',',  // 0xb3
        '.',  // 0xb4
        '/',  // 0xb5
        0,  // 0xb6 - Right Shift
        '*',  // 0xb7 - Keypad *
        0,  // 0xb8 - Left Alt
        ' ',  // 0xb9
        0,  // 0xba - Caps Lock
        0,  // 0xbb - F1
        0,  // 0xbc - F2
        0,  // 0xbd - F3
        0,  // 0xbe - F4
        0,  // 0xbf - F5
        0,  // 0xc0 - F6
        0,  // 0xc1 - F7
        0,  // 0xc2 - F8
        0,  // 0xc3 - F9
        0,  // 0xc4 - F10
        0,  // 0xc5 - Num Lock
        0,  // 0xc6 - Scroll Lock
        '7',  // 0xc7 - Keypad 7 / Home
        '8',  // 0xc8 - Keypad 8 / Up
        '9',  // 0xc9 - Keypad 9 / Page Up
        '-',  // 0xca - Keypad - 
        '4',  // 0xcb - Keypad 4 / Left
        '5',  // 0xcc - Keypad 5
        '6',  // 0xcd - Keypad 6 / Right
        '+',  // 0xce - Keypad +
        '1',  // 0xcf - Keypad 1 / End
        '2',  // 0xd0 - Keypad 2 / Down
        '3',  // 0xd1 - Keypad 3 / Page Down
        '0',  // 0xd2 - Keypad 0 / Insert
        '.',  // 0xd3 - Keypad . / Delete
        0,  // 0xd4 - Alt + SysReq (????)
        0,  // 0xd5
        0,  // 0xd6
        0,  // 0xd7 - F11
        0,  // 0xd8 - F12

        0,  // 0xd9
        0,  // 0xda
        0,  // 0xdb
        0,  // 0xdc
        0,  // 0xdd
        0,  // 0xde
        0,  // 0xdf
        0,  // 0xe0 - Two byte scan code
        0,  // 0xe1
        0,  // 0xe2
        0,  // 0xe3
        0,  // 0xe4
        0,  // 0xe5
        0,  // 0xe6
        0,  // 0xe7
        0,  // 0xe8
        0,  // 0xe9
        0,  // 0xea
        0,  // 0xeb
        0,  // 0xec
        0,  // 0xed
        0,  // 0xee
        0,  // 0xef
        0,  // 0xf0
        0,  // 0xf1
        0,  // 0xf2
        0,  // 0xf3
        0,  // 0xf4
        0,  // 0xf5
        0,  // 0xf6
        0,  // 0xf7
        0,  // 0xf8
        0,  // 0xf9
        0,  // 0xfa
        0,  // 0xfb
        0,  // 0xfc
        0,  // 0xfd
        0,  // 0xfe
    };

    /**
     * @brief Bool arrays, true for a key if it is in the made state
     * 
     */
    bool made[0x100] = {0};
    bool e0_made[0x100] = {0};

    bool lock[8] = {0};

    inline bool shift_active() {
        return (made[LEFT_SHIFT] || made[RIGHT_SHIFT]);
    }

    inline bool ctrl_active() {
        return (made[LEFT_CTRL] || e0_made[RIGHT_CTRL]);
    }

    inline bool alt_active() {
        return (made[LEFT_ALT] || e0_made[RIGHT_ALT]);
    }

    void key_make(unsigned char key) {

        char to_write = 0;

        if (lock[E0]) {
            e0_made[key] = true;
            lock[E0] = false;

        } else if (key == 0x3a) {
            lock[CAPS] = !lock[CAPS];    
            update_lights();
        
        } else {
            made[key] = true;

            // Check for shift key being pressed
            if (shift_active()) 
                to_write = shift_translation[key];
            else
                to_write = code_translation[key];

            // Check if Caps lock is active and applies
            if (lock[CAPS] && is_alphabet(to_write)) {
                if (to_write > 'Z')
                    to_write -= 32;
                else
                    to_write += 32;
            }
        }

        // Update the active terminal if necessary
        if (to_write != 0) {
            active_terminal->send_key(to_write);
        }
    }

    void key_break(unsigned char key) {

        if (key == 0xe0) {
            lock[E0] = true;
            return;
        }

        key -= 0x80;

        if (lock[E0]) {
            e0_made[key] = false;
            lock[E0] = false;

        } else {
            made[key] = false;
        }
    }

    void update_lights() {
        // Send command, and wait for confirmation
        out_byte(0xed, KB_CMD);
        io_wait();

        // Send light data
        unsigned char light_data = (lock[SCROLL] << SCROLL | lock[NUM] << NUM | lock[CAPS] << CAPS);
        out_byte(light_data, KB_CMD);
        io_wait();
    }

    void wait_ack() {
        while(in_byte(KB_CMD) != ACK) {}
    }

    kb_handler::kb_handler(size_t buffer_size) {

        // Prepare keyboard buffer
        buffer_start = (char*) malloc(buffer_size);
        buffer_next = buffer_start;
        *buffer_next = '\0';
        buffer_end = buffer_start + buffer_size;
    }

    void kb_handler::buffer_hard_clear() {

        fill_mem(buffer_start, (size_t)(buffer_end - buffer_start), 0);
        buffer_next = buffer_start;
    }

    void kb_handler::run_action(char target) {

        switch (key_actions[(unsigned char)target].code) {

            case TO_BUFFER:
                buffer_write_c(target);
                break;

            case BUFFER_SIGNAL:
                buffer_write_c(target);
                // fallthrough

            case SEND_SIGNAL:
                signal[key_actions[(unsigned char)target].signal](this, target);
                break;

            case HARD_CLEAR:
                buffer_hard_clear();
                break;

            case CLEAR_BUFFER:
                buffer_clear();
                break;

            case NO_ACTION:
                // fallthrough
            default:
                break;
        }

        return;
    }
}

#pragma GCC diagnostic pop
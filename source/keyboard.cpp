/**
 * @file keyboard.cpp
 * @author Shane Menzies
 * @brief Keyboard functions
 * @date 03/16/21
 * 
 * 
 */

#include "kb_codes.h"

namespace keyboard {

    /**
     * @brief Translation of keyboard scan code set 1
     * 
     */
    char code_translation[0xff] = {
        0,  // 0x00 - non-US-1
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
        0,  // 0x54 - Alt + SysReq (????)
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


    };
}
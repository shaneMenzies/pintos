#ifndef KEYBOARD_H
#define KEYBOARD_H

#pragma GCC diagnostic ignored "-Wchar-subscripts"

#include <stdint.h>
#include <stddef.h>

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

    enum key_action_codes : unsigned char {

        TO_BUFFER = 0,
        NO_ACTION = 1,

        BUFFER_SIGNAL = 0x0e,
        SEND_SIGNAL = 0x0f,

        HARD_CLEAR = 0xfe,
        CLEAR_BUFFER = 0xff,
    };

    struct key_action {
        unsigned char signal;
        key_action_codes code;

        inline key_action(key_action_codes code = TO_BUFFER, unsigned char signal = 0) : signal(signal), code(code) {};
    };

    class kb_handler {

        private:
            char* buffer_start;
            char* buffer_next;
            char* buffer_end;

            key_action key_actions[0x80] = {key_action()};
            void (*signal[0x100])(kb_handler* handler, char character) = {0};
            void (*on_buffer_write)(char character) = 0;

        public:
            kb_handler(size_t buffer_size = 512);

            inline void buffer_write_c(char target) {

                if (target == '\b') {
                    if (buffer_next > buffer_start)
                        buffer_next--;
                    *buffer_next = '\0';
                } else {
                    *buffer_next = target;
                    buffer_next++;
                    if (buffer_next > buffer_end) 
                        buffer_next = (buffer_start);
                    *buffer_next = '\0';
                }

                if ((uintptr_t)on_buffer_write != 0) {
                    on_buffer_write(target);
                }
            }

            inline void buffer_clear() {
                buffer_next = buffer_start;
                *buffer_next = '\0';
            }

            void buffer_hard_clear();
            inline const char* get_buffer() {
                return (const char*)buffer_start;
            }

            inline void set_action(char target, key_action action = key_action(TO_BUFFER,0)) {
                key_actions[target] = action;
            }

            inline void set_on_buffer_write(void (*new_handler)(char character)) {
                on_buffer_write = new_handler;
            }

            inline void clear_action(char target) {
                key_actions[target] = key_action(TO_BUFFER,0);
            }

            inline void set_signal(unsigned char index, void (*new_signal)(kb_handler* handler, char character)) {
                signal[index] = new_signal;
            }

            inline void clear_signal(unsigned char index) {
                signal[index] = 0;
            }

            void run_action(char target);
    };
}

#pragma GCC diagnostic pop

#endif
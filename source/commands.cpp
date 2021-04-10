/**
 * @file commands.cpp
 * @author Shane Menzies
 * @brief Kernel command line system
 * @date 04/10/21
 * 
 * 
 */

#include "commands.h"

namespace kernel {

    keyboard::kb_handler* cmd_handler = 0;

    void cmd_init() {

        // Create the command line keyboard handler
        cmd_handler = new keyboard::kb_handler();

        // Set up the special actions for the command-line
        cmd_handler->set_signal(0xff, send_command);
        cmd_handler->set_action('\n', keyboard::key_action(keyboard::SEND_SIGNAL, 0xff));
    }

    keyboard::kb_handler* get_cmd_handler() {

        return cmd_handler;
    }

    void send_command(keyboard::kb_handler* handler, char target) {

        // Get the command
        const char* cmd_buffer = handler->get_buffer();

        // TODO: GET ANY ADDITIONAL ARGUMENTS AND PASS THEM THROUGH

        // Parse the command
        int result = parse_command(cmd_buffer);

        // Clear the buffer, and put this character in
        handler->buffer_clear();
        handler->buffer_write_c(target);
    }

    int parse_command(const char* command) {
        active_terminal->write_s(command);
        return 0;
    }
}

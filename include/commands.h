#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdarg.h>

#include "keyboard.h"

namespace kernel {

    void cmd_init();

    keyboard::kb_handler* get_cmd_handler();

    void send_command(keyboard::kb_handler* handler, char target);

    int parse_command(const char* command);

}

#include "terminal.h"

#endif
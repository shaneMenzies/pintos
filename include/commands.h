#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdarg.h>

#include "keyboard.h"
#include "libk.h"

namespace kernel {

    void cmd_init();

    keyboard::kb_handler* get_cmd_handler();

    void send_command(keyboard::kb_handler* handler, char target);

    int parse_command(const char* command);

    struct command_entry : public pattern_entry<char> {

        command_entry(pattern_entry<char> pattern, int (*command_function)(int argc, char** argv)) 
            : pattern_entry<char>(pattern), command_function(command_function) {};

        int (*command_function)(int argc, char* argv[]);

    };

    void new_command(const char* identifier, int (*command)(int argc, char** argv));
    void batch_new_command(const char* identifier[], int (*command[])(int argc, char** argv), unsigned int num_commands);

    int run_command(const char* input, int argc, char* argv[]);

    namespace commands {
        int echo(int argc, char* argv[]);
        int test(int argc, char* argv[]);
    }
}

#include "terminal.h"

#endif
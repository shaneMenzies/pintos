/**
 * @file commands.cpp
 * @author Shane Menzies
 * @brief Kernel command line system
 * @date 04/10/21
 *
 *
 */

#include "commands.h"

#include "io/keyboard.h"
#include "libk/cstring.h"
#include "libk/misc.h"
#include "libk/sorting.h"
#include "terminal.h"
#include "threading/threading.h"

namespace kernel {

keyboard::kb_handler* cmd_handler = 0;

unsigned int    num_commands    = 0;
unsigned int    max_commands    = 0;
command_entry** command_entries = 0;

constexpr unsigned int num_kernel_commands = 5;
const char*            kernel_command_identifiers[num_kernel_commands]
    = {"echo", "test", "cpuinfo", "test_alloc", "branch"};
int (*kernel_command_pointers[num_kernel_commands])(int argc, char** argv)
    = {commands::echo, commands::test, commands::cpuinfo, commands::test_alloc,
       commands::branch};

void cmd_init() {

    // Create the command line keyboard handler
    cmd_handler = new keyboard::kb_handler();

    // Set up the special actions for the command-line
    cmd_handler->set_signal(0xff, send_command);
    cmd_handler->set_action('\n',
                            keyboard::key_action(keyboard::SEND_SIGNAL, 0xff));

    // Prepare default command entries
    max_commands = 256;
    command_entries
        = (command_entry**)malloc(sizeof(command_entry*) * max_commands);
    batch_new_command(kernel_command_identifiers, kernel_command_pointers,
                      num_kernel_commands);
}

keyboard::kb_handler* get_cmd_handler() { return cmd_handler; }

void send_command(keyboard::kb_handler* handler, char target) {

    (void)target;

    // Get the command
    const char* cmd_buffer = handler->get_buffer();

    // Parse the command
    int result = parse_command(cmd_buffer);

    // Clear the buffer
    handler->buffer_clear();
}

int parse_command(const char* command) {

    // Need to split the command into three parts:
    //      1. Target Command
    //      2. Number of Arguments
    //      3. Array of Arguments

    size_t command_size = std_k::strlen(command);

    char* target_command = (char*)malloc(command_size);
    std_k::strcpy(target_command, command);

    // Count the number of arguments
    int num_args = 0;
    for (size_t i = 0; i < command_size; i++) {
        if (target_command[i] == ' ') {
            // Space indicates another argument coming afterwards
            target_command[i] = '\0';
            i++;
            if (i < command_size) { num_args++; }
        } else if (target_command[i] == '\\') {
            i++;
        }
    }

    // Get and fill the array of arguments
    char** arguments = 0;
    if (num_args) {
        arguments = (char**)malloc(sizeof(char*) * num_args);

        // Find previously placed null characters to fill arguments
        int next_arg = 0;
        for (size_t i = 0; i < command_size; i++) {
            if (target_command[i] == '\0') {
                i++;
                arguments[next_arg++] = &(target_command[i]);
                if (next_arg == num_args) { break; }
            }
        }
    }

    // Now able to run command
    return run_command(target_command, num_args, arguments);
}

void new_command(const char* identifier,
                 int (*command)(int argc, char** argv)) {

    if (num_commands < max_commands) {
        // Add new command
        command_entry* target_command
            = new command_entry(std_k::cstring_to_pattern(identifier), command);
        command_entries[num_commands] = target_command;
        num_commands++;

        // Resort the command list
        std_k::insertion_sort_pointer<command_entry>(command_entries,
                                                     num_commands);
    }
}

void batch_new_command(const char* identifier[],
                       int (*command[])(int argc, char** argv),
                       unsigned int num_new_commands) {

    // Add all of the commands
    for (unsigned int i = 0; i < num_new_commands; i++) {
        if (num_commands < max_commands) {
            // Add new command
            command_entry* target_command = new command_entry(
                std_k::cstring_to_pattern(identifier[i]), command[i]);
            command_entries[num_commands] = target_command;
            num_commands++;
        }
    }

    // Resort command list
    std_k::insertion_sort_pointer<command_entry>(command_entries, num_commands);
}

int run_command(const char* input, int argc, char* argv[]) {

    int index = std_k::binary_match_pointer<std_k::pattern_entry<char>>(
        (std_k::pattern_entry<char>**)command_entries, num_commands,
        std_k::cstring_to_pattern(input));

    if (index < 0) {
        active_terminal->tprintf("Unknown command: \"%s\".\n", input);
        return -1;
    } else {
        return command_entries[index]->command_function(argc, argv);
    }
}

namespace commands {

int echo(int argc, char* argv[]) {
    if (argc > 0) {
        for (int i = 0; i < argc; i++) {
            active_terminal->write_s(argv[i]);
            active_terminal->write_c(' ');
        }
        active_terminal->write_c('\n');
        return 0;

    } else {
        return 1;
    }
}

int test(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    active_terminal->write_s("Test Successful. \n\n");
    return 0;
}

int cpuinfo(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    active_terminal->tprintf(
        "Total Threads: %u\nTotal Cores: %u\nTotal Sockets: %u\nTotal Domains: "
        "%u\n\n",
        topology.num_logical, topology.num_physical, topology.num_sockets,
        topology.num_domains);
    return 0;
}

int test_alloc(int argc, char* argv[]) {
    if (argc > 0) {
        malloc(std_k::string_to_number(argv[0]));
        return 0;
    } else {
        return 1;
    }
}

int branch(int argc, char* argv[]) {

    if (argc > 0) {
        auto wrapper = [](char* arg) { parse_command(arg); };
        std_k::preset_function<void(char*)> task(wrapper, argv[0]);
        threading::main_scheduler.send_task(
            new threading::process(2, 1, 1, 0x1000, &task));
        return 0;
    } else {
        return 1;
    }
}
} // namespace commands
} // namespace kernel

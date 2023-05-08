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
#include "memory/chunking.h"
#include "terminal.h"
#include "threading/process_def.h"
#include "threading/threading.h"

namespace kernel {

keyboard::kb_handler* cmd_handler = 0;

unsigned int    num_commands    = 0;
unsigned int    max_commands    = 0;
command_entry** command_entries = 0;

constexpr unsigned int num_kernel_commands = 8;
const char*            kernel_command_identifiers[num_kernel_commands]
    = {"echo",   "test",     "cpu_stat",  "test_alloc",
       "branch", "mem_stat", "proc_stat", "scheduling"};
int (*kernel_command_pointers[num_kernel_commands])(int argc, char** argv)
    = {commands::echo,       commands::test,      commands::cpu_stat,
       commands::test_alloc, commands::branch,    commands::mem_stat,
       commands::proc_stat,  commands::scheduling};

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

    char* target_command = new char[command_size];
    std_k::strcpy(target_command, command);

    if (command_size == 0) { return 0; }

    // Count the number of arguments
    int num_args = 1;
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
    char** arguments = new char*[num_args];

    // Find previously placed null characters to fill arguments
    arguments[0] = target_command;
    int next_arg = 1;
    for (size_t i = 0; i < command_size; i++) {
        if (target_command[i] == '\0') {
            i++;
            arguments[next_arg++] = &(target_command[i]);
            if (next_arg == num_args) { break; }
        }
    }

    // Now able to run command
    int result = run_command(target_command, num_args, arguments);

    delete[] target_command;
    delete[] arguments;

    return result;
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

int cpu_stat(int argc, char* argv[]) {
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
        size_t size = std_k::string_to_number(argv[1]);
        active_terminal->tprintf("Allocating %u(0x%x) bytes.\n",
                                 (unsigned int)size, (unsigned int)size);

        void* allocated = malloc(size);
        active_terminal->tprintf("Received memory at %p.\n", allocated);
        free(allocated);
        return 0;
    } else {
        return 1;
    }
}

int branch(int argc, char* argv[]) {

    if (argc < 2) { return 1; }

    size_t child_cmd_size = 0;
    for (int i = 1; i < argc; i++) {
        child_cmd_size += std_k::strlen(argv[i]);
        child_cmd_size++;
    }

    char*  child_command = new char[child_cmd_size];
    size_t cmd_index     = 0;
    for (int i = 1; i < argc;) {
        std_k::strcpy(&child_command[cmd_index], argv[i]);
        cmd_index += std_k::strlen(argv[i]);

        i++;
        if (i == argc) break;

        child_command[cmd_index] = ' ';
        cmd_index++;
    }
    child_command[cmd_index] = '\0';

    auto wrapper_func = [](char* command) {
        parse_command(command);
        delete command;
    };

    std_k::preset_function<void(const char*)>* task
        = (std_k::preset_function<void(const char*)>*)new std_k::
            preset_function<void(char*)>(wrapper_func, child_command);
    threading::system_scheduler.add_process(new threading::process(1, 1, task));
    return 0;
}

int mem_stat(int argc, char* argv[]) {

    active_terminal->tprintf("Free Memory:\n");
    size_t total = 0;

    // Print for each CPU thread
    active_terminal->tprintf("\tPre-allocated by Processor:\n");
    for (unsigned int i = 0; i < topology.num_logical; i++) {
        // Total of all pre-allocated chunks
        size_t current_total = 0;
        for (unsigned int j = 0; j < chunking::NUM_MEMORY_PILES; j++) {
            current_total += (topology.threads[i].memory_piles[j].chunk_size
                              * topology.threads[i].memory_piles[j].size());
        }

        active_terminal->tprintf("\t\tThread #%u - 0x%x bytes\n", i,
                                 current_total);
        total += current_total;
    }
    active_terminal->tprintf("\t\tTotal Pre-allocated - 0x%x bytes\n\n", total);

    // Print reservoir contents
    size_t reservoir_total = 0;
    for (unsigned int i = 0; i < chunking::NUM_MEMORY_PILES; i++) {
        reservoir_total += (chunking::memory_reservoirs[i].chunk_size
                            * chunking::memory_reservoirs[i].size());
    }
    active_terminal->tprintf("\tAvailable in Reservoirs - 0x%x bytes\n",
                             reservoir_total);
    total += reservoir_total;

    active_terminal->tprintf("Total Free: 0x%x bytes\n", total);

    return 0;
}

int proc_stat(int argc, char* argv[]) {

    // Print what each processor is doing
    active_terminal->tprintf("Processor State:\n");
    for (unsigned int i = 0; i < topology.num_logical; i++) {
        logical_core* current_thread = &topology.threads[i];

        active_terminal->tprintf("\tThread #%u - ", i);
        if (current_thread->scheduler->current_task == nullptr) {
            active_terminal->tprintf("Idle\n");
        } else {
            active_terminal->tprintf(
                "Active (pid = %u)\n",
                current_thread->scheduler->current_task->pid);
        }
    }

    // Print scheduler run queue
    active_terminal->tprintf("\nWaiting Processes:\n");
    for (unsigned int i = 0; i < threading::system_scheduler.size(); i++) {
        active_terminal->tprintf(
            "\t#%u - pid %u\n", i,
            threading::system_scheduler.run_queue.base[i]->pid);
    }

    return 0;
}

int scheduling(int argc, char* argv[]) {
    // Needs at least 1 argument
    if (argc < 2) {
        active_terminal->tprintf("No keyword provided.\n");
        return 1;
    }

    // Split on provided keyword
    if (std_k::strncmp(argv[1], "pause", 5) == 0) {
        // Pauses the system scheduler, keeping it from handing processes
        //  out to individual threads
        threading::system_scheduler.paused = true;
        return 0;

    } else if (std_k::strncmp(argv[1], "start", 5) == 0) {
        // Unpauses the system scheduler
        threading::system_scheduler.paused = false;
        return 0;

    } else if (std_k::strncmp(argv[1], "status", 6) == 0) {
        // More detailed information on the scheduler states

        // Print thread states
        active_terminal->tprintf("Thread Schedulers:\n");
        for (unsigned int i = 0; i < topology.num_logical; i++) {
            logical_core* current_thread = &topology.threads[i];

            active_terminal->tprintf("\tThread #%u: \n", i);

            if (current_thread->scheduler->current_task == nullptr) {
                active_terminal->tprintf("\t\tIdle\n");
            } else {
                active_terminal->tprintf(
                    "\t\tActive (pid = %u)\n",
                    current_thread->scheduler->current_task->pid);
            }

            active_terminal->tprintf(
                "\t\tAPIC (id = %u):\n",
                current_thread->scheduler->local_timer->id.id);
            active_terminal->tprintf(
                "\t\t\t# of Tasks: %u\n",
                current_thread->scheduler->local_timer->num_tasks());
            active_terminal->tprintf(
                "\t\t\tRate: %u hz\n",
                current_thread->scheduler->local_timer->apic_rate);
            active_terminal->tprintf(
                "\t\t\tCurrent time: %u ticks\n",
                current_thread->scheduler->local_timer->now());
            active_terminal->tprintf(
                "\t\t\tTime till next event: %u ticks\n",
                current_thread->scheduler->local_timer->time_to_next());
        }

        // Print system state
        active_terminal->tprintf("System Scheduler:\n");
        active_terminal->tprintf("\tLock State: ");
        active_terminal->tprintf((threading::system_scheduler.lock.is_locked()
                                      ? "True\n"
                                      : "False\n"));
        active_terminal->tprintf("\tPaused: ");
        active_terminal->tprintf(
            (threading::system_scheduler.paused ? "True\n" : "False\n"));
        active_terminal->tprintf("\tWaiting Processes:\n");
        for (unsigned int i = 0; i < threading::system_scheduler.size(); i++) {
            active_terminal->tprintf(
                "\t\t#%u - pid %u\n", i,
                threading::system_scheduler.run_queue.base[i]->pid);
        }

        return 0;

    } else {
        active_terminal->tprintf("Unrecognized keyword.\n");
        return 1;
    }
}
} // namespace commands
} // namespace kernel

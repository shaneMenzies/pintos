/**
 * @file error.cpp
 * @author Shane Menzies
 * @brief Functions around correctly handling errors and reporting them
 * @date 02/26/21
 *
 *
 */

#include "error.h"

#include "io/io.h"
#include "terminal/terminal.h"
#include "terminal/terminal_def.h"

visual_terminal* error_terminal = 0;

/**
 * @brief Public function to set the error terminal
 *
 * @param new_terminal  New error terminal
 */
void set_error_terminal(visual_terminal* new_terminal) {
    error_terminal = new_terminal;
}

/**
 * @brief Raises an error
 *
 * @param error_code    Error code
 * @param caller        Name of function raising the error
 */
void raise_error(uint32_t error_code = 0, const char* caller = 0) {

    // After error occurs, fallback to serial terminal
    io_write_s("Error Raised!", COM_1);

    // Print the associated info on this code
    io_write_s(const_cast<char*>(get_code_info(error_code)), COM_1);

}

/**
 * @brief Get the associated info for a certain error code
 *
 * @param error_code    Error code
 * @return char*        Null-terminated string containing the info
 */
const char* get_code_info(uint32_t error_code) {

    switch (error_code) {
        case 001: return "Invalid Inputs\n";

        case 002: return "Purely Virtual Function was called\n";

        case 003: return "General Protection Fault Encountered\n";

        case 004: return "Divide By Zero\n";

        case 005: return "Hardware error in memory module\n";

        case 006: return "Overflow exception\n";

        case 007: return "Bound range exceeded\n";

        case 8: return "Invalid Opcode\n";

        case 9: return "Device Not Available\n";

        case 10: return "Double Fault Exception\n";

        case 11: return "Segmentation Fault\n";

        case 12: return "Paging Fault\n";

        case 13: return "Floating Point Exception\n";

        case 14:
            return "Unaligned memory access with alignment checking enabled.\n";

        case 201: return "No Memory Available\n";

        case 202: return "Requested Bookmark not found\n";

        case 203: return "Incorrect find for this tree's sorting target\n";

        case 204: return "New bookmark already present in tree\n";

        case 301: return "Outside of framebuffer range\n";

        case 666: return "Stack Manipulation Detected!\n";

        default: return "Unknown Error Code\n";
    }
}

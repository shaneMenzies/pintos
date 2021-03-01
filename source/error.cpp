/**
 * @file error.cpp
 * @author Shane Menzies
 * @brief Functions around correctly handling errors and reporting them
 * @date 02/26/21
 * 
 * 
 */

#include "error.h"

#include "terminal.h"

uint32_t* error_code_addr = reinterpret_cast<uint32_t*>(0);
char** error_caller_addr = reinterpret_cast<char**>(0 + sizeof(uint32_t));

terminal* error_terminal = 0;

/**
 * @brief Public function to set the error terminal
 * 
 * @param new_terminal  New error terminal
 */
void set_error_terminal(terminal* new_terminal) {
    error_terminal = new_terminal;
}

/**
 * @brief Verifies that the error terminal exists
 * 
 */
void check_error_terminal() {
    
    // If no error terminal has been set, then make one
    if (error_terminal == 0) {
        error_terminal = new terminal(0xffffff, 0, 0x0f);
    }
}

/**
 * @brief Raises an error
 * 
 * @param error_code    Error code
 * @param caller        Name of function raising the error
 */
void raise_error(uint32_t error_code=0, char* caller=0) {

    // Check to ensure the error terminal exists
    check_error_terminal();

    *error_code_addr = error_code;
    *error_caller_addr = caller;

    // Print the error code to the terminal
    const char error_code_format[] = "\nError No. %u in %s:\n\t";
    error_terminal->printf(const_cast<char*>(error_code_format), 
                           error_code, caller);

    // Print the associated info on this code
    error_terminal->write(const_cast<char*>(get_code_info(error_code)));

    error_terminal->show(0xff2b3d, 0x00, 0b00001100);
}

/**
 * @brief Get the associated info for a certain error code
 * 
 * @param error_code    Error code
 * @return char*        Null-terminated string containing the info
 */
const char* get_code_info(uint32_t error_code) {

    switch (error_code) {
        case 001:
            return "Invalid Inputs";

        case 002:
            return "Purely Virtual Function was called";

        case 201:
            return "No Memory Available";

        case 202:
            return "Requested Bookmark not found";

        case 203:
            return "Incorrect find for this tree's sorting target";

        case 204:
            return "New bookmark already present in tree";

        case 301:
            return "Outside of framebuffer range";

        default:
            return "Unknown Error Code";
    }

}
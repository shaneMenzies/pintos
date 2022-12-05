/**
 * @file libk.cpp
 * @author Shane Menzies
 * @brief Some freestanding C standard library functions
 * @date 02/14/21
 *
 *
 */

#include "libk/misc.h"

#include "libk/cstring.h"

#include <cpuid.h>
#include <stddef.h>

namespace std_k {

/**
 * @brief Get the vendor of the system
 *
 * @return sytem_vendor Detected vendor
 */
system_vendor get_vendor() {

    constexpr uint32_t intel_string[3] = {0x756e6547, 0x49656e69, 0x6c65746e};
    constexpr uint32_t amd_string[3]   = {0x68747541, 0x69746e65, 0x444d4163};

    uint32_t eax, ebx, ecx, edx;
    eax = 0;
    __cpuid(0, eax, ebx, ecx, edx);

    if (ebx == intel_string[0] && edx == intel_string[1]
        && ecx == intel_string[2]) {
        return intel;
    } else if (ebx == amd_string[0] && edx == amd_string[1]
               && ecx == amd_string[2]) {
        return amd;
    } else {
        return other;
    }
}

/**
 * @brief Reverses the bits of an unsigned integer
 *
 * @param target        Number to be reversed
 * @param size          Size of the number (in bits)
 * @return unsigned int Reverse of the number
 */
unsigned int reverse_bits(unsigned int target, size_t size) {

    unsigned int return_int = 0;

    for (size_t i = 0; i < size; i += 8) {
        return_int |= (bit_reverse_table[(target >> i) & 0xff] << i);
    }
    return return_int;
}

pattern_entry<char> cstring_to_pattern(const char* string) {

    int length = 0;
    while (1) {

        if (string[length] == '\0') {
            length++;
            break;
        }

        length++;
    }

    pattern_entry<char> pattern;
    pattern.length = length;
    pattern.data   = string;

    return pattern;
}

void printf(char* target_buffer, const char* format, ...) {

    // Start the optional arguments
    va_list args;
    va_start(args, format);

    vprintf(target_buffer, format, args);

    va_end(args);
}

void vprintf(char* target_buffer, const char* format, va_list args) {

    // Index for format string
    int f_index = 0;

    // Index for build string
    int b_index = 0;

    while (1) {
        char target_char = format[f_index];

        // Catch if char is a %, then get next char, and act on it
        if (target_char == '%') {

            f_index++;
            target_char = format[f_index];

            switch (target_char) {
                case 'd':
                    /* fall through */
                case 'i':
                    // SIGNED INTEGER TO DECIMAL CHARS
                    b_index += (stringify<int>(&target_buffer[b_index],
                                               va_arg(args, int), 10));
                    break;

                case 'u':
                    // UNSIGNED INTEGER TO DECIMAL CHARS
                    b_index += stringify(&target_buffer[b_index],
                                         va_arg(args, unsigned int), 10);
                    break;

                case 'o':
                    // UNSIGNED INTEGER TO OCTAL CHARS
                    b_index += stringify(&target_buffer[b_index],
                                         va_arg(args, unsigned int), 8);
                    break;

                case 'x':
                    // UNSIGNED INTEGER TO HEX CHARS
                    b_index += stringify(&target_buffer[b_index],
                                         va_arg(args, unsigned int), 16);
                    break;

                case 'b':
                    // UNSIGNED INTEGER TO BINARY
                    b_index += stringify(&target_buffer[b_index],
                                         va_arg(args, unsigned int), 2);
                    break;

                case 'c':
                    // PRINT THE CHAR
                    target_buffer[b_index] = (char)va_arg(args, int);
                    b_index++;
                    break;

                case 's': {
                    // A STRING
                    uint16_t temp_index = 0;
                    char*    string     = va_arg(args, char*);

                    // Loop until null terminator encountered
                    while (1) {

                        char string_char = string[temp_index];

                        if (string_char == '\0') { break; }

                        target_buffer[b_index] = string_char;
                        b_index++;
                        temp_index++;
                    }
                } break;

                case 'p':
                    // POINTER TO HEX DIGITS
                    b_index += stringify(&target_buffer[b_index],
                                         (uintptr_t)va_arg(args, int*), 16);
                    break;

                case 'n':
                    // STORE b_index TO POINTER PROVIDED
                    *(va_arg(args, int*)) = b_index;
                    break;

                case '%':
                    // PRINT A %
                    target_buffer[b_index] = '%';
                    b_index++;
                    break;

                default: break;
            }

            // By default just copy the character over
        } else {
            target_buffer[b_index] = target_char;
            b_index++;

            // Null termination ends loop
            if (target_char == '\0') { break; }
        }

        // Increment format index
        f_index++;
    }
}

unsigned int string_to_number(const char* source) {

    // Get source length and temporary buffer for reversed version
    size_t source_length   = strlen(source);
    char*  reversed_string = new char[source_length + 1];
    if (source_length == 0) {
        return 0;
    } else if (source_length == 1) {
        return (source[0] - 48);
    }

    // Determine base
    unsigned int base;
    if (source[0] == '0' && source_length > 2) {
        switch (source[1]) {
            // Binary
            case 'b':
                base = 2;
                source_length -= 2;
                break;

            // Hexadecimal
            case 'h':
                /* fallthrough */
            case 'x':
                base = 16;
                source_length -= 2;
                break;

            // Octal
            default:
                base = 8;
                source_length -= 1;
                break;
        }
    } else {
        // Decimal
        base = 10;
    }

    // Get reversed string
    strrev(reversed_string, source);

    // Work through the reversed string to determine value
    unsigned int total       = 0;
    unsigned int place_value = 1;
    unsigned int i           = 0;
    while (i < source_length) {
        if (reversed_string[i] > 96) {
            // Lower case alphabetical
            total += (reversed_string[i] - 87) * place_value;
        } else if (reversed_string[i] > 64) {
            // Upper case alphabetical
            total += (reversed_string[i] - 55) * place_value;
        } else {
            // Numerical
            total += (reversed_string[i] - 48) * place_value;
        }

        place_value *= base;
        i++;
    }

    return total;
}
} // namespace std_k

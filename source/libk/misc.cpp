/**
 * @file libk.cpp
 * @author Shane Menzies
 * @brief Some freestanding C standard library functions
 * @date 02/14/21
 *
 *
 */

#include "libk/misc.h"

#include "cstring.h"
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

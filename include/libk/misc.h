#ifndef PINT_LIBK_H
#define PINT_LIBK_H

#include "common.h"
#include "cstring.h"

namespace std_k {

template<size_t size> class bitset {
  private:
    uint8_t data[(size / sizeof(uint8_t)) + ((size % sizeof(uint8_t)) ? 1 : 0)];

  public:
    bool get(const size_t index) const {

        size_t data_offset = (index / 8);
        size_t bit_offset  = (index % 8);
        return (data[data_offset] & (1 << bit_offset));
    };

    void set(const size_t index, bool value) {

        size_t data_offset = (index / 8);
        size_t bit_offset  = (index % 8);

        if (value)
            data[data_offset] |= (1 << bit_offset);
        else
            data[data_offset] &= ~(1 << bit_offset);
    };
};

template<class T> struct pattern_entry {

    size_t   length;
    const T* data;

    T           operator[](size_t index) { return data[index]; };
    friend bool operator==(pattern_entry& lhs, pattern_entry& rhs) {
        if (lhs.length != rhs.length) {
            return false;
        } else {
            for (size_t i = 0; i < lhs.length; i++) {
                if (lhs[i] != rhs[i]) { return false; }
            }
            return true;
        }
    }
    friend bool operator!=(pattern_entry& lhs, pattern_entry& rhs) {
        return !(lhs == rhs);
    }

    friend bool operator<(pattern_entry& lhs, pattern_entry& rhs) {
        bool   left_short = lhs.length < rhs.length;
        size_t min_length = (left_short) ? lhs.length : rhs.length;

        for (size_t i = 0; i < min_length; i++) {
            if (lhs[i] < rhs[i]) {
                return true;
            } else if (lhs[i] > rhs[i]) {
                return false;
            }
        }

        return left_short;
    }
    friend bool operator>(pattern_entry& lhs, pattern_entry& rhs) {
        return rhs < lhs;
    }
    friend bool operator<=(pattern_entry& lhs, pattern_entry& rhs) {
        return !(lhs > rhs);
    }
    friend bool operator>=(pattern_entry& lhs, pattern_entry& rhs) {
        return !(lhs < rhs);
    }
};

static constexpr unsigned char bit_reverse_table[]
    = {0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0,
       0x30, 0xB0, 0x70, 0xF0, 0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
       0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 0x04, 0x84, 0x44, 0xC4,
       0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
       0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC,
       0x3C, 0xBC, 0x7C, 0xFC, 0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
       0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 0x0A, 0x8A, 0x4A, 0xCA,
       0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
       0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6,
       0x36, 0xB6, 0x76, 0xF6, 0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
       0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE, 0x01, 0x81, 0x41, 0xC1,
       0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
       0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9,
       0x39, 0xB9, 0x79, 0xF9, 0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
       0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5, 0x0D, 0x8D, 0x4D, 0xCD,
       0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
       0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3,
       0x33, 0xB3, 0x73, 0xF3, 0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
       0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB, 0x07, 0x87, 0x47, 0xC7,
       0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
       0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF,
       0x3F, 0xBF, 0x7F, 0xFF};

inline bool is_alphabet(char character) {
    return ((character >= 'A' && character <= 'Z')
            || (character >= 'a' && character <= 'z'));
}

inline bool is_numeric(char character) {
    return (character >= '0' && character <= '9');
}

inline bool is_alphanumeric(char character) {
    return (is_alphabet(character) || is_numeric(character));
}

enum system_vendor : int {
    other = -1,
    amd   = 0,
    intel = 1,
};

system_vendor get_vendor();

unsigned int reverse_bits(unsigned int target, size_t size);

inline uint64_t round_next_binary_power(uint64_t input) {
    input--;
    input |= input >> 1;
    input |= input >> 2;
    input |= input >> 4;
    input |= input >> 8;
    input |= input >> 16;
    input |= input >> 32;
    input++;
    return input;
};

inline uint32_t round_next_binary_power(uint32_t input) {
    input--;
    input |= input >> 1;
    input |= input >> 2;
    input |= input >> 4;
    input |= input >> 8;
    input |= input >> 16;
    input++;
    return input;
};

inline uint8_t round_next_binary_power(uint8_t input) {
    input--;
    input |= input >> 1;
    input |= input >> 2;
    input |= input >> 4;
    input++;
    return input;
};

inline uint64_t round_up_multiple(uint64_t input, uint64_t multiple) {
    return ((input + multiple - 1) & (-multiple));
};
inline uint64_t round_down_multiple(uint64_t input, uint64_t multiple) {
    return (input - (input % multiple));
};

template<typename T> inline T log2(T value) {
    return ((sizeof(T) * 8) - __builtin_clz(value) - 1);
}

pattern_entry<char> cstring_to_pattern(const char* string);

void printf(char* target_buffer, const char* format, ...);

void vprintf(char* target_buffer, const char* format, va_list args);

unsigned int string_to_number(const char* source);

/**
 * @brief Converts a number into characters in the target char array
 *
 * @param target_buffer     Char buffer for output to be placed in
 * @param number            Number to convert
 * @param base              Base to interpret number as
 * @return uint16_t         Chars placed
 */
template<typename T>
unsigned int stringify(char* target_buffer, T number, uint8_t base) {

    unsigned int index = 0;
    // Allocate some memory to temporarily store the digits of the number in
    char temp_string[64];

    // Flag to set if number was negative
    bool negative = false;

    // If the number is negative (and signed), put a - in front and make it
    // positive
    if (is_signed<T>() && number < 0) {
        negative = true;
        number   = -number;
    }

    // Loop through placing digits until the number is gone
    while (1) {
        // Get the digit
        char digit = number % base;
        number /= base;

        // Convert digit to corresponding ascii code
        if (digit < 10) {
            // Numerical characters
            digit += 48;
        } else {
            // Alphabetical characters
            // (87 instead of 97, simplified from (digit - 10) + 97)
            digit += 87;
        }

        // Place the digit
        temp_string[index] = digit;

        // Break if number is 0
        if (number <= 0) {
            break;
        } else {
            // Else increment the index
            index++;
            if (index >= 62) break;
        }
    }

    if (negative) { temp_string[++index] = '-'; }
    temp_string[++index] = '\0';

    // temp_string now contains the digits, but they're backwards
    strrev(target_buffer, temp_string);

    // Return the number of chars placed
    return (index);
}

} // namespace std_k

#endif

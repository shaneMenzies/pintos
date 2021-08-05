/**
 * @file libk.cpp
 * @author Shane Menzies
 * @brief Some freestanding C standard library functions
 * @date 02/14/21
 * 
 * 
 */

#include "libk.h"
#include "p_memory.h"

/**
 * @brief Get the vendor of the system
 * 
 * @return sytem_vendor Detected vendor 
 */
system_vendor get_vendor() {

    const uint32_t intel_string[3] = {
        0x756e6547, 0x49656e69, 0x6c65746e
    };
    const uint32_t amd_string[3] = {
        0x68747541, 0x69746e65, 0x444d4163
    };

    uint32_t eax, ebx, ecx, edx;
    eax = 0;
    __cpuid(0, eax, ebx, ecx, edx);

    if (ebx == intel_string[0] && edx == intel_string[1] && ecx == intel_string[2]) {
        return intel;
    } else if (ebx == amd_string[0] && edx == amd_string[1] && ecx == amd_string[2]) {
        return amd;
    } else {
        return other;
    }
}

/**
 * @brief Copies the contents of one area of memory to another
 * 
 * @param dest_ptr  Pointer to the start of the destination to be copied to
 * @param src_ptr   Pointer to the start of the source to be copied from
 * @param size      Size (in bytes) of area to be copied
 * @return void*    dest_ptr
 */
void* memcpy(void* __restrict__ dest_ptr, const void* __restrict__ src_ptr, 
             size_t size) {

    unsigned int* dest_mword = (unsigned int*) dest_ptr;
    unsigned int* src_mword = (unsigned int*) src_ptr;

    // Fill out most of the data as quickly as possible
    int i = 0;
    for (; size >= sizeof(unsigned int); i++, size -= sizeof(unsigned int)) {
        dest_mword[i] = src_mword[i];
    }
    dest_mword = &dest_mword[i];
    src_mword = &src_mword[i];

    // If it's not entirely filled out, fill the last bits
    if (size > 0) {
        unsigned int fill_data = *src_mword;
        fill_data <<= (8*size);
        *dest_mword &= ~fill_data;
        *dest_mword |= fill_data;
    }

    return dest_ptr;
}

/**
 * @brief Fills an entire section of memory with a certain fill pattern
 * 
 * @param dest_ptr  Area to be filled
 * @param size      Size of area to be filled
 * @param fill_data Data to be filled with
 */
void fill_mem(void* dest_ptr, size_t size, unsigned long int fill_data) {

    unsigned long int* dest_mword = (unsigned long int*) dest_ptr;

    // Fill out most of the data as quickly as possible
    unsigned int i = 0;
    for (; size >= sizeof(unsigned long int); i++) {
        dest_mword[i] = fill_data;
        size -= sizeof(unsigned long int);
    }
    dest_mword = &(dest_mword[i]);

    // If it's not entirely filled out, fill the last bits
    if (size > 0) {
        for (unsigned int x = 0; x < size; x++) {
            ((unsigned char*)dest_mword)[x] = ((unsigned char*)(&fill_data))[x];
        }
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

/**
 * @brief Compares two strings, and returns true if they are equivalent, and 
 *        false otherwise.
 * 
 * @param first     First string for comparison
 * @param second    Second string for comparison
 * @return true     Strings are equal
 * @return false    Strings are different
 */
bool str_cmp(const char* first, const char* second) {

    int i = 0;
    while (1) {
        if (first[i] == '\0' && second[i] == '\0') {
            return true;
        } else if (first[i] != second[i]) {
            return false;
        } else {
            i++;
        }
    }
}

/**
 * @brief Gets the size of a c-string
 * 
 * @param string    String to get the size of (null-terminated)
 * @return size_t   Total size of the string (includes null character)
 */
size_t str_size(const char* string) {

    int i = 0;
    while (1) {
        if (string[i++] == '\0') {
            return i;
        }
    }
}

void str_copy(char* destination, const char* source) {

    int i = 0;
    while (1) {
        destination[i] = source[i];
        if (source[i] == '\0') {
            return;
        } else {
            i++;
        }
    }
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
    pattern.data = string;

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
                        char* string = va_arg(args, char*);

                        // Loop until null terminator encountered
                        while (1) {

                            char string_char = string[temp_index];

                            if (string_char == '\0') {
                                break;
                            }

                            target_buffer[b_index] = string_char;
                            b_index++;
                            temp_index++;
                        }
                    }
                    break;

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

                default:
                    break;
            }

        // By default just copy the character over
        } else {
            target_buffer[b_index] = target_char;
            b_index++;

            // Null termination ends loop
            if (target_char == '\0') {
                break;
            }
        }

        // Increment format index
        f_index++;
    }
}

/**
 * @file cstring.cpp
 * @author Shane Menzies
 * @brief Cstring functions
 * @date 05/26/22
 *
 *
 */

#include "libk/cstring.h"

#include "libk/common.h"

namespace std_k {

/**
 * @brief Gets the size of a c-string
 *
 * @param str       String to get the size of (null-terminated)
 * @return size_t   Total size of the string (without null character)
 */
size_t strlen(const char* str) {

    int i = 0;
    while (1) {
        if (str[i] == '\0') { return i; }
        i++;
    }
}

/**
 * @brief Copies one string(src) to another(dest), including null terminator
 *
 * @param destination Destination buffer
 * @param source      Source string (null-terminated)
 * @return char*
 */
char* strcpy(char* __restrict__ destination, const char* __restrict source) {

    int i = 0;
    while (1) {
        destination[i] = source[i];
        if (source[i] == '\0') {
            return destination;
        } else {
            i++;
        }
    }

    return destination;
}

/**
 * @brief Compares two strings, and returns the difference between the
 * first pair of different characters found.
 *
 * @param lhs     First string for comparison
 * @param rhs     Second string for comparison
 * @return int    Difference between first pair of different characters found.
 */
int strcmp(const char* lhs, const char* rhs) {

    int i = 0;
    while (1) {
        if (lhs[i] == '\0' && rhs[i] == '\0') {
            return 0;
        } else if (lhs[i] != rhs[i]) {
            return (rhs[i] - lhs[i]);
        } else {
            i++;
        }
    }
}

/**
 * @brief Copies a string (src) to another (dest), reversing the order of
 *          characters (tomorrow becomes worromot).
 *          Null terminator stays at end of string.
 *
 * @param dest  Destination buffer
 * @param src   Source string (null-terminated)
 */
char* strrev(char* dest, const char* src) {
    if (dest == src) {
        // In place reverse
        size_t length = strlen(src) - 1;
        for (size_t i = 0; i <= length; i++) {
            char temp          = dest[i];
            dest[i]            = dest[(length - i)];
            dest[(length - i)] = temp;
        }
        dest[length + 1] = '\0';

        return dest;
    } else {
        // Normal reverse
        size_t s_index = (strlen(src) - 1);
        size_t t_index = 0;

        while (1) {
            dest[t_index] = src[s_index];

            if (s_index == 0) { break; }

            t_index += 1;
            s_index -= 1;
        }

        dest[t_index + 1] = '\0';
        return dest;
    }
}

/**
 * @brief Copies specified number of characters from one string(src) to
 * another(dest)
 *
 * @param destination Destination buffer
 * @param source      Source string (null-terminated)
 * @param n           Number of characters to copy
 * @return char*
 */
char* strncpy(char* destination, const char* source, size_t n) {

    size_t i = 0;
    while (1) {
        destination[i] = source[i];
        if (source[i] == '\0') {
            return destination;
        } else {
            i++;
        }
    }

    while (i < n) {
        destination[i] = '\0';
        i++;
    }

    return destination;
}

/**
 * @brief Compares the first n characters of two strings, and returns the
 * difference between the first pair of different characters found.
 *
 * @param lhs     First string for comparison
 * @param rhs     Second string for comparison
 * @param n       Number of characters to compare
 * @return int    Difference between first pair of different characters found.
 */
int strncmp(const char* lhs, const char* rhs, size_t n) {

    size_t i = 0;
    while (i < n) {
        if (lhs[i] == '\0' && rhs[i] == '\0') {
            return 0;
        } else if (lhs[i] != rhs[i]) {
            return (rhs[i] - lhs[i]);
        } else {
            i++;
        }
    }
    return 0;
}

/**
 * @brief Copies the first n characters of one string (src) to another (dest),
 * reversing the order of characters (tomorrow becomes worromot). Null
 * terminator stays at end of string.
 *
 * @param dest  Destination buffer
 * @param src   Source string (null-terminated)
 * @param n     Number of characters to reverse
 */
char* strnrev(char* dest, const char* src, size_t n) {

    if (dest == src) {
        // In-place reverse
        size_t length = n - 1;

        // Reversed portion
        for (size_t i = 0; i <= length; i++) {
            char temp          = dest[i];
            dest[i]            = dest[(length - i)];
            dest[(length - i)] = temp;
        }

        // Normal portion not needed, it hasn't been changed
        return dest;
    } else {
        size_t s_index = n - 1;
        size_t t_index = 0;

        // Reverse portion
        while (1) {
            dest[t_index] = src[s_index];

            if (s_index == 0) { break; }

            t_index += 1;
            s_index -= 1;
        }

        // Normal portion
        s_index = n;
        while (src[s_index] != '\0') {
            dest[s_index] = src[s_index];
            s_index++;
        }

        dest[s_index + 1] = '\0';
        return dest;
    }
}

/**
 * @brief Fills an entire section of memory with a certain character
 *
 * @param dest      Area to be filled
 * @param ch        Character to fill with
 * @param count     Number of characters to set
 */
void* memset(void* dest, int ch, size_t count) {

    for (size_t i = 0; i < count; i++) {
        ((unsigned char*)dest)[i] = (unsigned char)ch;
    }

    return dest;
}

/**
 * @brief Fills an entire section of memory with a certain value
 *
 * @param dest      Area to be filled
 * @param value     Value to fill with
 * @param count     Number of values to set
 */
template<class T> void* memset_t(void* dest, T value, size_t count) {

    for (size_t i = 0; i < count; i++) { ((T*)dest)[i] = value; }

    return dest;
}

/**
 * @brief Copies the contents of one area of memory to another
 *
 * @param dest  Pointer to the start of the destination to be copied to
 * @param src   Pointer to the start of the source to be copied from
 * @param count Size (in bytes) of area to be copied
 * @return void*    dest
 */
void* memcpy(void* __restrict__ dest, const void* __restrict__ src,
             size_t count) {

    for (size_t i = 0; i < count; i++) {
        ((unsigned char*)dest)[i] = ((unsigned char*)src)[i];
    }

    return dest;
}

/**
 * @brief Moves the contents of one area of memory to another
 *
 * @param dest  Pointer to the start of the destination to be moved to
 * @param src   Pointer to the start of the source to be moved from
 * @param count Size (in bytes) of area to be copied
 * @return void*    dest
 */
void* memmove(void* dest, const void* src, size_t count) {

    for (size_t i = 0; i < count; i++) {
        ((unsigned char*)dest)[i] = ((unsigned char*)src)[i];
    }

    return dest;
}

/**
 * @brief Compares two areas of memory, and returns the difference between the
 * first pair of different characters found.
 *
 * @param lhs     First region for comparison
 * @param rhs     Second region for comparison
 * @return int    Difference between first pair of different characters found.
 */
int memcmp(const void* lhs, const void* rhs, size_t count) {

    for (size_t i = 0; i < count; i++) {
        if (((unsigned char*)lhs)[i] != ((unsigned char*)rhs)[i]) {
            return (int)(((unsigned char*)rhs)[i] - ((unsigned char*)lhs)[i]);
        }
    }

    return 0;
}

} // namespace std_k

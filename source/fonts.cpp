/**
 * @file fonts.cpp
 * @author Shane Menzies
 * @brief Font handling
 * @date 03/22/21
 * 
 * 
 */

#include "fonts.h"

Font::Font(const unsigned int char_width, const unsigned int char_height, 
    const unsigned int char_count, const unsigned int* bit_map)
    : bit_map(bit_map), char_width(char_width),  
    char_height(char_height), char_count(char_count), 
    char_offset((size_t)(sizeof(unsigned int) * char_height))
    {}

unsigned int* Font::get_char(char target) const {
    unsigned int* return_addr = (unsigned int*)((uintptr_t)bit_map + (target * char_offset));
    return return_addr;
}

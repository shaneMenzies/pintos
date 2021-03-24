#ifndef FONTS_H
#define FONTS_H

#include <stdint.h>
#include <stddef.h>

class Font {

    private:
        const unsigned int* bit_map;

    public:

        const int char_width;
        const int char_height;
        const int char_count;
        const size_t char_offset;

        Font(const int char_width, const int char_height, 
            const int char_count, const unsigned int* bit_map);

        unsigned int* get_char(char target);
};

#endif
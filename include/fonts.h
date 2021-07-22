#ifndef FONTS_H
#define FONTS_H

#include <stdint.h>
#include <stddef.h>

class Font {

    private:
        const unsigned int* bit_map;

    public:

        const unsigned int char_width;
        const unsigned int char_height;
        const unsigned int char_count;
        const size_t char_offset;

        Font(const unsigned int char_width, const unsigned int char_height, 
            const unsigned int char_count, const unsigned int* bit_map);

        unsigned int* get_char(char target) const;
};

#endif
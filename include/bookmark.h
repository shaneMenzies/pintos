#ifndef BOOKMARK_H
#define BOOKMARK_H

namespace trees {

    class bookmark {

        public:

            bookmark(){};
            bookmark(void* start, void* end);
            bookmark(void* start, size_t size);

            void* start;
            void* end;
            size_t size;
            bookmark* link[2];
            bookmark* parent;
            char balance;
            unsigned char flags;

            inline bool has_left();
            inline bool has_right();
            inline bool has_both();
            inline bool has_any();

            inline bool is_left();
            inline bool is_right();

            inline void set_right(bookmark* new_child);
            inline void set_left(bookmark* new_child);

            inline bookmark* get_right();
            inline bookmark* get_left();
            inline bookmark* get_parent();

            inline void remove_right();
            inline void remove_left();
            inline void remove_any();
    };

    enum mark_flags {
        MARK_FREE = 1,
        LEFT_CHILD = (1<<1),
        RIGHT_CHILD = (1<<2),
        LOCKED_MARK = (1<<4),
        IS_RIGHT_CHILD = (1<<5),
        UNDEFINED_MARK_FLAGS = (1<<3) | (1<<6) | (1<<7)
    };

    enum link {
        RIGHT = 1,
        LEFT = 0
    };
}

#endif
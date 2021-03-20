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

            bool has_left();
            bool has_right();
            bool has_both();
            bool has_any();

            bool is_left();
            bool is_right();

            void set_right(bookmark* new_child);
            void set_left(bookmark* new_child);

            bookmark* get_right();
            bookmark* get_left();
            bookmark* get_parent();

            void remove_right();
            void remove_left();
            void remove_any();
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
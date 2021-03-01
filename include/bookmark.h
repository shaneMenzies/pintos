#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <stddef.h>
#include <stdint.h>

typedef struct bookmark {

    void* start;
    void* end;
    size_t size;
    bookmark* link[2];
    bookmark* parent;
    char balance;
    unsigned char flags;

} bookmark;

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

class mark_tree {
    private:
        bookmark* root_mark;
        bool size_sorted = false;

        // Hard find for finding a certain bookmark by it's address,
        // in a tree sorted by size, should be avoided if possible
        bookmark* hard_find(void*);

        // Simple rotations around a certain parent node
        void right(bookmark*);
        void left(bookmark*);

        // Simple rotations around the root node
        void right();
        void left();

        // Double rotations around a certain parent node
        void right_left(bookmark*);
        void left_right(bookmark*);

        // Double rotations around the root node
        void right_left();
        void left_right();

        // Function that seperates a certain bookmark from the
        // rest of the tree
        void seperate(bookmark*);

        // Function that balances tree from certain mark up,
        // it does apply the balance change itself
        void balance_ancestors(bookmark*, int8_t);

    public:
        mark_tree(bookmark*);

        bookmark* find(void*);
        bookmark* find(void*, size_t);
        bookmark* find_suitable(size_t);

        void insert(bookmark*);
        void remove(void*);
        void remove(void*, size_t);
};

#endif
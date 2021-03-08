#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <stddef.h>
#include <stdint.h>

#include "error.h"

namespace trees {

    class bookmark {

        public:

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

            bool is_left();
            bool is_right();

            void set_right(bookmark* new_child);
            void set_left(bookmark* new_child);

            bookmark* get_right();
            bookmark* get_left();
            bookmark* get_parent();

            void remove_right();
            void remove_left();
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

    class mark_tree {
        private:

            // Hard find for finding a certain bookmark by it's address,
            // in a tree sorted by size, should be avoided if possible
            bookmark* hard_find(void*);

            // Simple rotations around a certain parent node
            bookmark* right(bookmark*);
            bookmark* left(bookmark*);

            // Simple rotations around the root node
            bookmark* right();
            bookmark* left();

            // Double rotations around a certain parent node
            bookmark* right_left(bookmark*);
            bookmark* left_right(bookmark*);

            // Double rotations around the root node
            bookmark* right_left();
            bookmark* left_right();

        public:
            bookmark* root_mark = 0;

            bool size_sorted = false;

            mark_tree(bool = false);
            mark_tree(bookmark*, bool = false, bool = false);

            unsigned int get_height();

            // Search functions
            bookmark* find(void*);
            bookmark* find(void*, size_t);
            bookmark* find_suitable(size_t);

            // Simple modifications
            void insert(bookmark*);
            void remove(void*);
            void remove(void*, size_t);

            // Function that seperates a certain bookmark from the
            // rest of the tree
            void seperate(bookmark*);
            void seperate_root();

            // Functions that balances the tree after a change
            void balance_insertion(bookmark*, int8_t);
            void balance_deletion(bookmark*, int8_t);

    };

    // Set operations on entire trees
    mark_tree join_right(mark_tree left, unsigned int left_height, bookmark* joiner,
                        mark_tree right, unsigned int right_height);
    mark_tree join_left(mark_tree left, unsigned int left_height, bookmark* joiner,
                        mark_tree right, unsigned int right_height);
    mark_tree join(mark_tree left, bookmark* joiner, mark_tree right);

    void split_size(mark_tree& left_tree, mark_tree& right_tree, 
            mark_tree old_tree, bookmark* key);
    void split_addr(mark_tree& left_tree, mark_tree& right_tree, 
            mark_tree old_tree, bookmark* key);

    void split(mark_tree& left_tree, mark_tree& right_tree, 
            mark_tree old_tree, bookmark* key);

    mark_tree tree_union(mark_tree tree_one, mark_tree tree_two);

}

#endif
/**
 * @file bookmark.cpp
 * @author Shane Menzies
 * @brief Bookmark system for memory management, using AVL binary search trees
 * @date 02/27/21
 * 
 * 
 */

#include "bookmark.h"

#include "error.h"

/**
 * @brief Construct a new AVL mark tree
 * 
 * @param init_mark     Pointer to the bookmark to start as the root of the tree
 */
mark_tree::mark_tree(bookmark* init_mark) : root_mark(init_mark) {
    root_mark->parent = NULL;
    root_mark->flags &= ~(LEFT_CHILD | RIGHT_CHILD);
    root_mark->balance = 0;
}

/* #region search_functions */

/**
 * @brief Recursively searchs through the entire tree to find a certain
 *        mark based on its start address. Very slow compared to other methods,
 *        searching by address and size should be chosen instead if at all
 *        possible.
 * 
 * @param target_start  start value for the mark to be found
 * @return bookmark*    Pointer to the mark with the requested address,
 *                      or a 0 if no mark could be found
 */
bookmark* mark_tree::hard_find(void* target_start) {

    // Start at root mark
    bookmark* current_mark = root_mark;

    // Flag for use in looping
    bool reverse = false;

    // Search left hand side
    while (1) {

        // Check if this is the target mark
        if (current_mark->start == target_start) {
            return current_mark;
        }

        // Has the reverse flag been set?
        if (reverse) {

            reverse = false;
            // Does this mark have a right child?
            if (current_mark->flags & RIGHT_CHILD) {
                current_mark = current_mark->link[RIGHT];
                continue;
            } else {

                // Reached the end of this path, so go up until we're
                // at the end of the right children, before going to the 
                // parent's right child
                while (current_mark->flags & IS_RIGHT_CHILD) {
                    current_mark = current_mark->parent;
                }
                current_mark = current_mark->parent;
                reverse = true;
            }

        // If not going reverse, look at the left child
        } else {

            // Does this mark have a left child?
            if (current_mark->flags & LEFT_CHILD) {
                current_mark = current_mark->link[LEFT];
                continue;
            } else {
                // Reached the end of this path, so go up 1, and to the right child
                current_mark = current_mark->parent;
                reverse = true;
            }
        }

    }

    // Search right hand side
    while (1) {

        // Check if this is the target mark
        if (current_mark->start == target_start) {
            return current_mark;
        }

        // Has the reverse flag been set?
        if (reverse) {

            reverse = false;
            // Does this mark have a left child?
            if (current_mark->flags & LEFT_CHILD) {
                current_mark = current_mark->link[LEFT];
                continue;
            } else {

                // Reached the end of this path, so go up until we're
                // at the end of the left children, before going to the 
                // parent's left child
                while (!(current_mark->flags & IS_RIGHT_CHILD)) {
                    current_mark = current_mark->parent;
                }
                current_mark = current_mark->parent;
                reverse = true;
            }

        // If not going reverse, look at the right child
        } else {

            // Does this mark have a right child?
            if (current_mark->flags & RIGHT_CHILD) {
                current_mark = current_mark->link[RIGHT];
                continue;
            } else {
                // Reached the end of this path, so go up 1, and to the left child
                current_mark = current_mark->parent;
                reverse = true;
            }
        }

    }

    // If it still hasn't been found, then it probably doesn't exist, so
    // raise an error and return 0
    raise_error(202, const_cast<char*>("mark_tree::hard_find(void*)"));
    return 0;
}

/**
 * @brief Finds the mark with a certain starting address in it's info
 * 
 * @param target_start   start value for the mark to be found
 * @return bookmark*    Pointer to the mark with the requested address,
 *                      or a 0 if no mark could be found
 */
bookmark* mark_tree::find(void* target_start) {

    // If the tree is sorted by size, and we only know it's starting
    // address, resort to the hard_find function
    if (size_sorted) {
        return hard_find(target_start);
    }

    bookmark* current_mark = root_mark;

    while (current_mark->start != target_start) {

        // Move to one of the child nodes
        if ((current_mark->start < target_start) 
             && (current_mark->flags & RIGHT_CHILD)) {
            current_mark = current_mark->link[1];
        } else if ((current_mark->start > target_start) 
                    && (current_mark->flags & LEFT_CHILD)) {
            current_mark = current_mark->link[0];
        } else {
            // No valid link, so there is no valid mark with this size
            raise_error(202, const_cast<char*>("mark_tree::find(void*)"));
            return 0;
        }
    }

    return current_mark;
}

/**
 * @brief Ideal find function for a tree sorted by size, finds the mark with
 *        the requested start value and size
 * 
 * @param target_start  start value for the mark to be found
 * @param size          size of the mark to be found
 * @return bookmark*    Pointer to the requested bookmark, or a 0
 *                      if no matching mark could be found
 */
bookmark* mark_tree::find(void* target_start, size_t size) {

    // If the tree is NOT sorted by size, then this is the wrong function
    if (size_sorted) {
        raise_error(203, const_cast<char*>("mark_tree::find(void*, size_t)"));
        return 0;
    }

    bookmark* current_mark = root_mark;

    while (1) {

        // Check the size
        if ((current_mark->size <= size) 
             && (current_mark->flags & RIGHT_CHILD)) {

            // Catch if size is equal, check if the address is the same
            if ((current_mark->size == size) 
                 && (current_mark->start == target_start)) {
                     return current_mark;
            } else {
                // If this isn't the right mark, then travel to the right
                current_mark = current_mark->link[RIGHT];
            }

        } else if ((current_mark->size > size) 
                    && (current_mark->flags & LEFT_CHILD)) {
            current_mark = current_mark->link[LEFT];
        } else {
            // No valid link, so there is no valid mark with this size
            raise_error(202, const_cast<char*>("mark_tree::find(void*)"));
            return 0;
        }
    }

    return current_mark;
}

/**
 * @brief Finds the smallest suitable mark for a certain sized request.
 * 
 * @param min_size      Minimum size of mark to find.
 * @return bookmark*    Pointer to the suitable bookmark, or 0 if no
 *                      suitable mark could be found.
 */
bookmark* mark_tree::find_suitable(size_t min_size) {

    // Ensure that this tree is sorted by size
    if (!size_sorted) {
        raise_error(203, const_cast<char*>("mark_tree::find_suitable(size_t)"));
        return 0;
    }

    // Pointer to store the last suitable mark's address in
    bookmark* last_suitable = 0;

    // Start at the root mark
    bookmark* current_mark = root_mark;

    while (1) {

        // If this mark would be suitable, save it
        if (current_mark->size >= min_size) {
            last_suitable = current_mark;
        }

        // Jump to the next mark
        if ((current_mark->size < min_size) 
             && (current_mark->flags & RIGHT_CHILD)) {
            current_mark = current_mark->link[1];
        } else if ((current_mark->size > min_size) 
                    && (current_mark->flags & LEFT_CHILD)) {
            current_mark = current_mark->link[0];
        } else {
            // Either this mark is perfect (in which case it's already been
            // saved in last_suitable), or there are no more nodes
            // to search through, so it's time to stop looping
            break;
        }
    }

    // Check if no suitable mark was found, 
    // if so raise an error before returning
    if (last_suitable == 0) {
        raise_error(202, const_cast<char*>("mark_tree::find_suitable(size_t)"));
    }

    return last_suitable;
}

/* #endregion search_functions */

/* #region tree_rotations */

void mark_tree::right(bookmark* parent) {
    
    // Store the address of the child, and then rotate the child into its place
    bookmark* child_mark = parent->link[LEFT];
    child_mark->parent = parent->parent;
    if (parent->flags & IS_RIGHT_CHILD) {
        parent->parent->link[RIGHT] = child_mark;
        child_mark->flags |= IS_RIGHT_CHILD;
    } else {
        parent->parent->link[LEFT] = child_mark;
        child_mark->flags &= ~(IS_RIGHT_CHILD);
    }

    // If the child itself has a right child, then rotate that too
    if (child_mark->flags & RIGHT_CHILD) {
        parent->link[LEFT] = child_mark->link[RIGHT];
        parent->link[LEFT]->parent = parent;
        parent->link[LEFT]->flags &= ~(IS_RIGHT_CHILD);
    } else {
        // If it doesn't already have a child, then it does now,
        // and the parent won't have left child
        child_mark->flags |= RIGHT_CHILD;
        parent->flags &= ~(LEFT_CHILD);
    }

    // Set the child to be the new parent
    parent->parent = child_mark;
    parent->flags |= IS_RIGHT_CHILD;

    // Update balance factors
    parent->balance += (1 - child_mark->balance);
    child_mark->balance += 1;
}

void mark_tree::left(bookmark* parent) {

    // Store the address of the child, and then rotate the child into its place
    bookmark* child_mark = parent->link[RIGHT];
    child_mark->parent = parent->parent;
    if (parent->flags & IS_RIGHT_CHILD) {
        parent->parent->link[RIGHT] = child_mark;
        child_mark->flags |= IS_RIGHT_CHILD;
    } else {
        parent->parent->link[LEFT] = child_mark;
        child_mark->flags &= ~(IS_RIGHT_CHILD);
    }

    // If the child itself has a left child, then rotate that too
    if (child_mark->flags & LEFT_CHILD) {
        parent->link[RIGHT] = child_mark->link[LEFT];
        parent->link[RIGHT]->parent = parent;
        parent->link[RIGHT]->flags |= IS_RIGHT_CHILD;
    } else {
        // If it doesn't already have a child, then it does now,
        // and the parent won't have left child
        child_mark->flags |= LEFT_CHILD;
        parent->flags &= ~(RIGHT_CHILD);
    }

    // Set the child to be the new parent
    parent->parent = child_mark;
    parent->flags &= ~(IS_RIGHT_CHILD);

    // Update balance factors
    parent->balance -= (1 - child_mark->balance);
    child_mark->balance -= 1;
}

void mark_tree::right() {
    
    // Store the address of the marks, and then rotate the child into its place
    bookmark* parent_mark = root_mark;
    root_mark = parent_mark->link[LEFT];
    root_mark->parent = NULL;
    root_mark->flags &= ~(IS_RIGHT_CHILD);

    // If the child itself has a right child, then rotate that too
    if (root_mark->flags & RIGHT_CHILD) {
        parent_mark->link[LEFT] = root_mark->link[RIGHT];
        parent_mark->link[LEFT]->parent = parent_mark;
        parent_mark->link[LEFT]->flags &= ~(IS_RIGHT_CHILD);
    } else {
        // If it doesn't already have a child, then it does now,
        // and the parent won't have left child
        root_mark->flags |= RIGHT_CHILD;
        parent_mark->flags &= ~(LEFT_CHILD);
    }

    // Set the child to be the new parent
    parent_mark->parent = root_mark;
    parent_mark->flags |= IS_RIGHT_CHILD;

    // Update balance factors
    parent_mark->balance += (1 - root_mark->balance);
    root_mark->balance += 1;
}

void mark_tree::left() {

    // Store the address of the marks, and then rotate the child into its place
    bookmark* parent_mark = root_mark;
    root_mark = parent_mark->link[RIGHT];
    root_mark->parent = NULL;
    root_mark->flags &= ~(IS_RIGHT_CHILD);

    // If the child itself has a left child, then rotate that too
    if (root_mark->flags & LEFT_CHILD) {
        parent_mark->link[RIGHT] = root_mark->link[LEFT];
        parent_mark->link[RIGHT]->parent = parent_mark;
        parent_mark->link[RIGHT]->flags |= IS_RIGHT_CHILD;
    } else {
        // If it doesn't already have a child, then it does now,
        // and the parent won't have left child
        root_mark->flags |= LEFT_CHILD;
        parent_mark->flags &= ~(RIGHT_CHILD);
    }

    // Set the child to be the new parent
    parent_mark->parent = root_mark;
    parent_mark->flags &= ~(IS_RIGHT_CHILD);

    // Update balance factors
    parent_mark->balance -= (1 - root_mark->balance);
    root_mark->balance -= 1;
}

void mark_tree::right_left(bookmark* parent) {

    // Store the address of the child and grandchild, 
    // since they'll be needed a lot
    bookmark* child_mark = parent->link[RIGHT];
    bookmark* grandchild_mark = child_mark->link[LEFT];

    // Set the grandchild in place
    grandchild_mark->parent = parent->parent;
    if (parent->flags & IS_RIGHT_CHILD) {
        parent->parent->link[RIGHT] = grandchild_mark;
        grandchild_mark->flags |= IS_RIGHT_CHILD;
    } else {
        parent->parent->link[LEFT] = grandchild_mark;
        grandchild_mark->flags &= ~(IS_RIGHT_CHILD);
    }

    // Move the grandchild's children onto the parent and child, 
    // and then move down the parent and child to be the new
    // children of the current grandchild

    // Right Child
    if (grandchild_mark->flags & RIGHT_CHILD) {
        child_mark->link[LEFT] = grandchild_mark->link[RIGHT];
        child_mark->link[LEFT]->parent = child_mark;
        child_mark->link[LEFT]->flags &= ~(IS_RIGHT_CHILD);
    } else {
        grandchild_mark->flags |= RIGHT_CHILD;
        child_mark->flags &= ~(LEFT_CHILD);
    }
    grandchild_mark->link[RIGHT] = child_mark;
    child_mark->parent = grandchild_mark;
    child_mark->flags |= IS_RIGHT_CHILD;

    // Left Child
    if (grandchild_mark->flags & LEFT_CHILD) {
        parent->link[RIGHT] = grandchild_mark->link[LEFT];
        parent->link[RIGHT]->parent = parent;
        parent->link[RIGHT]->flags |= IS_RIGHT_CHILD;
    } else {
        grandchild_mark->flags |= LEFT_CHILD;
        parent->flags &= ~(RIGHT_CHILD);
    }
    grandchild_mark->link[LEFT] = parent;
    parent->parent = grandchild_mark;
    parent->flags &= ~(IS_RIGHT_CHILD);
    
    // Update balance factors
    parent->balance = -(grandchild_mark->balance);
    grandchild_mark->balance = 0;
    child_mark->balance = 0;
}

void mark_tree::left_right(bookmark* parent) {
    
    // Store the address of the child and grandchild, 
    // since they'll be needed a lot
    bookmark* child_mark = parent->link[LEFT];
    bookmark* grandchild_mark = child_mark->link[RIGHT];

    // Set the grandchild in place
    grandchild_mark->parent = parent->parent;
    if (parent->flags & IS_RIGHT_CHILD) {
        parent->parent->link[RIGHT] = grandchild_mark;
        grandchild_mark->flags |= IS_RIGHT_CHILD;
    } else {
        parent->parent->link[LEFT] = grandchild_mark;
        grandchild_mark->flags &= ~(IS_RIGHT_CHILD);
    }

    // Move the grandchild's children onto the parent and child, 
    // and then move down the parent and child to be the new
    // children of the current grandchild

    // Right Child
    if (grandchild_mark->flags & RIGHT_CHILD) {
        parent->link[LEFT] = grandchild_mark->link[RIGHT];
        parent->link[LEFT]->parent = parent;
        parent->link[LEFT]->flags &= ~(IS_RIGHT_CHILD);
    } else {
        grandchild_mark->flags |= RIGHT_CHILD;
        parent->flags &= ~(LEFT_CHILD);
    }
    grandchild_mark->link[RIGHT] = parent;
    parent->parent = grandchild_mark;
    parent->flags |= IS_RIGHT_CHILD;

    // Left Child
    if (grandchild_mark->flags & LEFT_CHILD) {
        child_mark->link[RIGHT] = grandchild_mark->link[LEFT];
        child_mark->link[RIGHT]->parent = child_mark;
        child_mark->link[RIGHT]->flags |= IS_RIGHT_CHILD;
    } else {
        grandchild_mark->flags |= LEFT_CHILD;
        child_mark->flags &= ~(RIGHT_CHILD);
    }
    grandchild_mark->link[LEFT] = child_mark;
    child_mark->parent = grandchild_mark;
    child_mark->flags &= ~(IS_RIGHT_CHILD);

    // Update balance factors
    child_mark->balance = -(grandchild_mark->balance);
    grandchild_mark->balance = 0;
    parent->balance = 0;

    // Update balance factors
    child_mark->balance = -(grandchild_mark->balance);
    grandchild_mark->balance = 0;
    parent->balance = 0;
}

void mark_tree::right_left() {

    // Store the address of the child and grandchild, 
    // since they'll be needed a lot
    // + for root oriented, need to store the parent's
    //   address for use even after the root_mark changes
    bookmark* parent_mark = root_mark;
    bookmark* child_mark = parent_mark->link[RIGHT];
    bookmark* grandchild_mark = child_mark->link[LEFT];

    // Set the grandchild in place
    root_mark = grandchild_mark;
    grandchild_mark->parent = NULL;
    grandchild_mark->flags &= ~(IS_RIGHT_CHILD);

    // Move the grandchild's children onto the parent and child, 
    // and then move down the parent and child to be the new
    // children of the current grandchild

    // Right Child
    if (grandchild_mark->flags & RIGHT_CHILD) {
        child_mark->link[LEFT] = grandchild_mark->link[RIGHT];
        child_mark->link[LEFT]->parent = child_mark;
        child_mark->link[LEFT]->flags &= ~(IS_RIGHT_CHILD);
    } else {
        grandchild_mark->flags |= RIGHT_CHILD;
        child_mark->flags &= ~(LEFT_CHILD);
    }
    grandchild_mark->link[RIGHT] = child_mark;
    child_mark->parent = grandchild_mark;
    child_mark->flags |= IS_RIGHT_CHILD;

    // Left Child
    if (grandchild_mark->flags & LEFT_CHILD) {
        parent_mark->link[RIGHT] = grandchild_mark->link[LEFT];
        parent_mark->link[RIGHT]->parent = parent_mark;
        parent_mark->link[RIGHT]->flags |= IS_RIGHT_CHILD;
    } else {
        grandchild_mark->flags |= LEFT_CHILD;
        parent_mark->flags &= ~(RIGHT_CHILD);
    }
    grandchild_mark->link[LEFT] = parent_mark;
    parent_mark->parent = grandchild_mark;
    parent_mark->flags &= ~(IS_RIGHT_CHILD);

    // Update balance factors
    parent_mark->balance = -(root_mark->balance);
    root_mark->balance = 0;
    child_mark->balance = 0;
}

void mark_tree::left_right() {
    
    // Store the address of the child and grandchild, 
    // since they'll be needed a lot
    // + for root oriented, need to store the parent's
    //   address for use even after the root_mark changes
    bookmark* parent_mark = root_mark;
    bookmark* child_mark = parent_mark->link[RIGHT];
    bookmark* grandchild_mark = child_mark->link[LEFT];

    // Set the grandchild in place
    root_mark = grandchild_mark;
    grandchild_mark->parent = NULL;
    grandchild_mark->flags &= ~(IS_RIGHT_CHILD);

    // Move the grandchild's children onto the parent and child, 
    // and then move down the parent and child to be the new
    // children of the current grandchild

    // Right Child
    if (grandchild_mark->flags & RIGHT_CHILD) {
        parent_mark->link[LEFT] = grandchild_mark->link[RIGHT];
        parent_mark->link[LEFT]->parent = parent_mark;
        parent_mark->link[LEFT]->flags &= ~(IS_RIGHT_CHILD);
    } else {
        grandchild_mark->flags |= RIGHT_CHILD;
        parent_mark->flags &= ~(LEFT_CHILD);
    }
    grandchild_mark->link[RIGHT] = parent_mark;
    parent_mark->parent = grandchild_mark;
    parent_mark->flags |= IS_RIGHT_CHILD;

    // Left Child
    if (grandchild_mark->flags & LEFT_CHILD) {
        child_mark->link[RIGHT] = grandchild_mark->link[LEFT];
        child_mark->link[RIGHT]->parent = child_mark;
        child_mark->link[RIGHT]->flags |= IS_RIGHT_CHILD;
    } else {
        grandchild_mark->flags |= LEFT_CHILD;
        child_mark->flags &= ~(RIGHT_CHILD);
    }
    grandchild_mark->link[LEFT] = child_mark;
    child_mark->parent = grandchild_mark;
    child_mark->flags &= ~(IS_RIGHT_CHILD);

    // Update balance factors
    child_mark->balance = -(root_mark->balance);
    root_mark->balance = 0;
    parent_mark->balance = 0;
}

void mark_tree::balance_ancestors(bookmark* current_mark, int8_t change) {

    current_mark->balance += change;

    while (current_mark->balance != 0) {
        // If balance factor >= 2 then right tree needs rebalancing
        if (current_mark->balance >= 2) {
            // Balance right tree

            // Is this the root node?
            bool root_node = false;
            if (current_mark == root_mark) {
                root_node = true;
            }

            // If the child is negative (reverse of parent),
            // then a right_left rotation is needed instead
            if (current_mark->link[RIGHT]->balance < 0) {
                if (root_node) {
                    right_left();
                } else {
                    right_left(current_mark);
                }
            } else {
                if (root_node) {
                    left();
                } else {
                    left(current_mark);
                }
            }

        // If balance factor <= -2 then left tree needs rebalancing
        } else if (current_mark->balance <= -2) {
            // Balance left tree

            // Is this the root node?
            bool root_node = false;
            if (current_mark == root_mark) {
                root_node = true;
            }

            // If the child is positive (reverse of parent),
            // then a left_right rotation is needed instead
            if (current_mark->link[RIGHT]->balance > 0) {
                if (root_node) {
                    left_right();
                } else {
                    left_right(current_mark);
                }
            } else {
                if (root_node) {
                    right();
                } else {
                    right(current_mark);
                }
            }
        }


        // Move to the parent, and update it's balance factor
        int8_t modifier = -1;
        if (current_mark->flags & IS_RIGHT_CHILD) {
            modifier = 1;
        }
        current_mark = current_mark->parent;
        current_mark->balance += modifier;
    }
}

/* #endregion tree_rotations */

/**
 * @brief Places a new mark into this tree, sorting it into the correct position
 *        based on either size or address. This function also does rebalance
 *        the tree after placing the new mark.
 *              - Note: In cases of sorting by size, where the size of the new
 *                      mark is the same as another one, the new mark will
 *                      always get placed on the RIGHT side of the old one
 * 
 * @param new_mark New mark to be placed in the tree
 */
void mark_tree::insert(bookmark* new_mark) {

    bookmark* current_mark = root_mark;
    int8_t balance_change;

    // Search through the tree to find the correct spot for the new mark
    while (1) {

        // Determine direction from current mark and what the tree
        // is sorted by (address or size)
        link direction = RIGHT;

        if (size_sorted) {
            // Sorted by size
            if (current_mark->size > new_mark->size) {
                direction = LEFT;

            // If current_mark->size <= new_mark->size, the direction 
            // isn't changed from RIGHT
            }
        } else {
            // Sorted by starting address
            if (current_mark->start > new_mark->start) {
                direction = LEFT;

            // If current_mark->start < new_mark->start, the direction 
            // isn't changed from RIGHT
            } else if (current_mark->start == new_mark->start) {
                // If the new start address is the same as one already
                // existing, something is very wrong, and most likely
                // this mark already exists.
                raise_error(204, const_cast<char*>("mark_tree::insert(bookmark*"));
                return;
            }
        }

        // If the direction doesn't have a child then put the new mark there
        // and break, otherwise just travel down that path
        if (direction == LEFT) {
            if (!(current_mark->flags & LEFT_CHILD)) {
                current_mark->flags |= LEFT_CHILD;
                current_mark->link[LEFT] = new_mark;
                balance_change = -1;
                new_mark->parent = current_mark;
                new_mark->flags &= ~(IS_RIGHT_CHILD);
                break;
            } else {
                current_mark = current_mark->link[LEFT];
            }
        } else {
            if (!(current_mark->flags & RIGHT_CHILD)) {
                current_mark->flags |= RIGHT_CHILD;
                current_mark->link[RIGHT] = new_mark;
                balance_change= 1;
                new_mark->parent = current_mark;
                new_mark->flags |= IS_RIGHT_CHILD;
                break;
            } else {
                current_mark = current_mark->link[RIGHT];
            }
        }
    }

    // Rebalance the tree
    balance_ancestors(current_mark, balance_change);
}

void mark_tree::remove(void* target_start) {

    // Find the target mark in the tree
    bookmark* target_mark = find(target_start);

    // Seperate the mark from the rest of the tree
    seperate(target_mark);
}

void mark_tree::remove(void* target_start, size_t size) {

    // Find the target mark in the tree, assisted by the size
    bookmark* target_mark = find(target_start, size);
    
    // Seperate the mark from the rest of the tree
    seperate(target_mark);
}

void mark_tree::seperate(bookmark* target_mark) {

    int8_t balance_change;

    // Check if the mark has children
    bool right_child = target_mark->flags & RIGHT_CHILD;
    bool left_child = target_mark->flags & LEFT_CHILD;

    // If it has no children, the process is very easy
    if (!(right_child) && !(left_child)) {

        if (target_mark->flags & IS_RIGHT_CHILD) {
            target_mark->parent->flags &= ~(RIGHT_CHILD);
            balance_change = -1;
        } else {
            target_mark->parent->flags &= ~(LEFT_CHILD);
            balance_change = 1;
        }

    // Only right child
    } else if (right_child && !left_child) {

        if (target_mark->flags & IS_RIGHT_CHILD) {
            target_mark->parent->link[RIGHT] = target_mark->link[RIGHT];
            balance_change = -1;
        } else {
            target_mark->parent->link[LEFT] = target_mark->link[RIGHT];
            target_mark->link[RIGHT]->flags &= ~(IS_RIGHT_CHILD);
            balance_change = 1;
        }

        target_mark->link[RIGHT]->parent = target_mark->parent;

    // Only left child
    } else if (left_child && !right_child) {

        if (target_mark->flags & IS_RIGHT_CHILD) {
            target_mark->parent->link[RIGHT] = target_mark->link[LEFT];
            target_mark->link[RIGHT]->flags |= IS_RIGHT_CHILD;
            balance_change = -1;
        } else {
            target_mark->parent->link[LEFT] = target_mark->link[LEFT];
            balance_change = 1;
        }

        target_mark->link[LEFT]->parent = target_mark->parent;

    // It has children on both sides (longest case)
    } else {



    }

    // Rebalance the tree
    balance_ancestors(target_mark, balance_change);
}

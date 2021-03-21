/**
 * @file trees.cpp
 * @author Shane Menzies
 * @brief Bookmark system for memory management, using AVL binary search trees
 * @date 02/27/21
 * 
 * 
 */

#include "trees.h"

namespace trees {

/* #region bookmark functions */

bookmark::bookmark(void* start_addr, void* end_addr) : start(start_addr), end(end_addr) {
    this->size = (size_t)((uintptr_t)end_addr - (uintptr_t)start_addr);
    this->balance = 0;
    this->flags = 0;
}

bookmark::bookmark(void* start_addr, size_t new_size) : start(start_addr), size(new_size) {
    this->end = (void*)((uintptr_t)start_addr + new_size);
    this->balance = 0;
    this->flags = 0;
}

/**
 * @brief Returns true if this mark has a left child, or false if it doesn't
 * 
 * @return true     If it does have a left child
 * @return false    If it doesn't have a left child
 */
bool bookmark::has_left() {
    if (this->flags & LEFT_CHILD) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief Returns true if this mark has a right child, or false if it doesn't
 * 
 * @return true     If it does have a right child
 * @return false    If it doesn't have a right child
 */
bool bookmark::has_right() {
    if (this->flags & RIGHT_CHILD) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief Returns true if this mark has both a right and left child,
 *        or false if it doesn't have both.
 * 
 * @return true     If it has a child on each side
 * @return false    If it doesn't have a child on each side
 */
bool bookmark::has_both() {
    if ((this->flags & (RIGHT_CHILD | LEFT_CHILD)) == (RIGHT_CHILD | LEFT_CHILD))
        return true;
    else 
        return false;
}

/**
 * @brief Returns true if this mark has any child,
 *        or false if it doesn't have any.
 * 
 * @return true     If it has a child on either side
 * @return false    If it doesn't have a child
 */
bool bookmark::has_any() {
    if (this->flags & (RIGHT_CHILD | LEFT_CHILD))
        return true;
    else 
        return false;
}

/**
 * @brief Returns true if this mark is a left child, or false if it isn't
 * 
 * @return true     If it is a left child
 * @return false    If it isn't a left child
 */
bool bookmark::is_left() {
    if (!(this->flags & IS_RIGHT_CHILD)) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief Returns true if this mark is a right child, or false if it isn't
 * 
 * @return true     If it is a right child
 * @return false    If it isn't a right child
 */
bool bookmark::is_right() {
    if (this->flags & IS_RIGHT_CHILD) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief Set's this mark's right child
 * 
 * @param new_child     New child
 */
void bookmark::set_right(bookmark* new_child) {
    this->link[RIGHT] = new_child;
    this->flags |= RIGHT_CHILD;
    new_child->parent = this;
    new_child->flags |= IS_RIGHT_CHILD;
}

/**
 * @brief Set's this mark's left child
 * 
 * @param new_child     New child
 */
void bookmark::set_left(bookmark* new_child) {
    this->link[LEFT] = new_child;
    this->flags |= LEFT_CHILD;
    new_child->parent = this;
    new_child->flags &= ~(IS_RIGHT_CHILD);
}

/**
 * @brief Gets this mark's right child
 * 
 * @return bookmark*    Right child
 */
bookmark* bookmark::get_right() {
    return this->link[RIGHT];
}

/**
 * @brief Gets this mark's left child
 * 
 * @return bookmark*    Left child
 */
bookmark* bookmark::get_left() {
    return this->link[LEFT];
}

/**
 * @brief Gets this mark's parent mark
 * 
 * @return bookmark*    Parent
 */
bookmark* bookmark::get_parent() {
    return this->parent;
}

/**
 * @brief Removes this mark's connection to it's right child
 * 
 */
void bookmark::remove_right() {
    this->flags &= ~(RIGHT_CHILD);
}

/**
 * @brief Removes this mark's connection to it's left child
 * 
 */
void bookmark::remove_left() {
    this->flags &= ~(LEFT_CHILD);
}
 
 /**
  * @brief Removes this mark's connections to any children
  * 
  */
void bookmark::remove_any() {
    this->flags &= !(LEFT_CHILD | RIGHT_CHILD);
}

/* #endregion */

/* #region mark_tree functions */

/**
 * @brief Construct a new, blank, tree. 
 * 
 * @param size_sorted   Whether the new tree is sorted by size or not
 */
mark_tree::mark_tree(bool size_sorted) : size_sorted(size_sorted){

}

/**
 * @brief Construct a new AVL mark tree, based off an already existing mark
 * 
 * @param init_mark     Pointer to the bookmark to start as the root of the tree
 */
mark_tree::mark_tree(bookmark* init_mark, bool size_sorted, bool in_form) : root_mark(init_mark) {
    root_mark->parent = NULL;

    if (size_sorted) {
        this->size_sorted = true;
    }

    if (!in_form) {
        root_mark->flags &= ~(LEFT_CHILD | RIGHT_CHILD);
        root_mark->balance = 0;
    }
}

/**
 * @brief Calculates the height of this tree, and returns it
 * 
 * @return unsigned int     Current height of this tree, at it's tallest point
 */
unsigned int mark_tree::get_height() {

    bookmark* current_mark = root_mark;

    // If tree not created yet, return 0
    if (current_mark == 0) {
        return 0;
    }

    unsigned int total_height = 0;

    while(1) {
        total_height++;

        // Only break out of this loop once there are no more children
        if (!current_mark->has_left() && !current_mark->has_right()) {
            break;
        }

        // Move based on balance of this mark
        if (current_mark->balance < 0) {
            current_mark = current_mark->link[LEFT];
        } else {
            current_mark = current_mark->link[RIGHT];
        }
    }

    return total_height;
}

/* #region search functions */

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
            if (current_mark->has_right()) {
                current_mark = current_mark->link[RIGHT];
                continue;
            } else {

                // Reached the end of this path, so go up until we're
                // at the end of the right children, before going to the 
                // parent's right child
                while (1) {
                    current_mark = current_mark->parent;

                    if (current_mark->is_left()) {
                        break;
                    }
                }

                // If we've returned to the root, then break
                if (current_mark == root_mark) {
                    break;
                }

                reverse = true;
            }

        // If not going reverse, look at the left child
        } else {

            // Does this mark have a left child?
            if (current_mark->flags & LEFT_CHILD) {
                current_mark = current_mark->link[LEFT];
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
            if (current_mark->has_left()) {
                current_mark = current_mark->link[LEFT];
            } else {

                // Reached the end of this path, so go up until we're
                // at the end of the left children, before going to the 
                // parent's left child
                while (1) {
                    current_mark = current_mark->parent;

                    if (current_mark->is_left()) {
                        break;
                    }
                }

                // If we've returned to the root, then break
                if (current_mark == root_mark) {
                    break;
                }

                reverse = true;
            }

        // If not going reverse, look at the right child
        } else {

            // Does this mark have a right child?
            if (current_mark->has_right()) {
                current_mark = current_mark->link[RIGHT];
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
        if ((current_mark->start < target_start) && current_mark->has_right()) {
            current_mark = current_mark->link[RIGHT];
        } else if ((current_mark->start > target_start) && current_mark->has_left()) {
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
        if ((current_mark->size <= size) && current_mark->has_right()) {

            // Catch if size is equal, check if the address is the same
            if ((current_mark->size == size) 
                 && (current_mark->start == target_start)) {
                     return current_mark;
            } else {
                // If this isn't the right mark, then travel to the right
                current_mark = current_mark->link[RIGHT];
            }

        } else if ((current_mark->size > size) && current_mark->has_left()) {
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
        if ((current_mark->size < min_size) && current_mark->has_right()) {
            current_mark = current_mark->link[RIGHT];
        } else if ((current_mark->size > min_size) && current_mark->has_left()) {
            current_mark = current_mark->link[LEFT];
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

/**
 * @brief Finds the bookmark which contains the requested address
 * 
 * @param target_addr   Address to find associated mark for
 * @return bookmark*    Bookmark containing the requested address
 */
bookmark* mark_tree::find_containing(void* target_addr) {

    // Start at root mark
    bookmark* current_mark = root_mark;

    // Flag for use in looping
    bool reverse = false;

    // Search left hand side
    while (1) {

        // Check if this is the target mark
        if (current_mark->start <= target_addr && current_mark->end >= target_addr) {
            return current_mark;
        }

        // Has the reverse flag been set?
        if (reverse) {

            reverse = false;
            // Does this mark have a right child?
            if (current_mark->has_right()) {
                current_mark = current_mark->link[RIGHT];
                continue;
            } else {

                // Reached the end of this path, so go up until we're
                // at the end of the right children, before going to the 
                // parent's right child
                while (1) {
                    current_mark = current_mark->parent;

                    if (current_mark->is_left()) {
                        break;
                    }
                }

                // If we've returned to the root, then break
                if (current_mark == root_mark) {
                    break;
                }

                reverse = true;
            }

        // If not going reverse, look at the left child
        } else {

            // Does this mark have a left child?
            if (current_mark->flags & LEFT_CHILD) {
                current_mark = current_mark->link[LEFT];
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
        if (current_mark->start <= target_addr && current_mark->end >= target_addr) {
            return current_mark;
        }

        // Has the reverse flag been set?
        if (reverse) {

            reverse = false;
            // Does this mark have a left child?
            if (current_mark->has_left()) {
                current_mark = current_mark->link[LEFT];
            } else {

                // Reached the end of this path, so go up until we're
                // at the end of the left children, before going to the 
                // parent's left child
                while (1) {
                    current_mark = current_mark->parent;

                    if (current_mark->is_left()) {
                        break;
                    }
                }

                // If we've returned to the root, then break
                if (current_mark == root_mark) {
                    break;
                }

                reverse = true;
            }

        // If not going reverse, look at the right child
        } else {

            // Does this mark have a right child?
            if (current_mark->has_right()) {
                current_mark = current_mark->link[RIGHT];
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

/* #endregion search_functions */

/* #region tree rotations */

/**
 * @brief Right-Right rotation around the provided parent mark
 * 
 * @param parent        Parent mark to be rotated around
 * @return bookmark*    New parent mark, after rotation
 */
bookmark* mark_tree::right(bookmark* parent) {
    
    // Store the addresses of the related marks,
    // and then rotate the child into its place.
    bookmark* grandparent_mark = parent->parent;
    bookmark* child_mark = parent->link[LEFT];

    if (parent->is_right()) {
        grandparent_mark->set_right(child_mark);
    } else {
        grandparent_mark->set_left(child_mark);
    }

    // If the child itself has a right child, then rotate that too
    if (child_mark->has_right()) {
        parent->set_left(child_mark->link[RIGHT]);
    } else {
        parent->remove_left();
    }

    // Child's right child is now the parent
    child_mark->set_right(parent);

    // Update balance factors
    if (child_mark->balance == 0) {
        parent->balance = -1;
        child_mark->balance = 1;
    } else {
        parent->balance = 0;
        child_mark->balance = 0;
    }

    // Return the new base mark
    return child_mark;
}


/**
 * @brief Left-Left rotation around the provided parent mark
 * 
 * @param parent        Parent mark to be rotated around
 * @return bookmark*    New parent mark, after rotation
 */
bookmark* mark_tree::left(bookmark* parent) {

    // Store the addresses of the related marks,
    // and then rotate the child into its place.
    bookmark* grandparent_mark = parent->parent;
    bookmark* child_mark = parent->link[RIGHT];

    if (parent->is_right()) {
        grandparent_mark->set_right(child_mark);
    } else {
        grandparent_mark->set_left(child_mark);
    }

    // If the child itself has a left child, then rotate that too
    if (child_mark->has_left()) {
        parent->set_right(child_mark->link[LEFT]);
    } else {
        parent->remove_right();
    }

    // Child's left child is now the parent
    child_mark->set_left(parent);

    // Update balance factors
    if (child_mark->balance == 0) {
        parent->balance = 1;
        child_mark->balance = -1;
    } else {
        parent->balance = 0;
        child_mark->balance = 0;
    }

    // Return the new base mark
    return child_mark;
}

/**
 * @brief Right-Right rotation around the root node
 * 
 * @return bookmark*    New Root Node
 */
bookmark* mark_tree::right() {
    
    // Store the addresses of the related marks,
    // and then rotate the child into its place.
    bookmark* parent = root_mark;
    root_mark = parent->link[LEFT];

    // If the child itself has a right child, then rotate that too
    if (root_mark->has_right()) {
        parent->set_left(root_mark->link[RIGHT]);
    } else {
        parent->remove_left();
    }

    // Child's right child is now the parent
    root_mark->set_right(parent);

    // Update balance factors
    if (root_mark->balance == 0) {
        parent->balance = -1;
        root_mark->balance = 1;
    } else {
        parent->balance = 0;
        root_mark->balance = 0;
    }

    // Return the new base mark
    return root_mark;
}

/**
 * @brief Left-Left rotation around the root node
 * 
 * @return bookmark*    New Root Node
 */
bookmark* mark_tree::left() {

    // Store the addresses of the related marks,
    // and then rotate the child into its place.
    bookmark* parent = root_mark;
    root_mark = parent->link[RIGHT];

    // If the child itself has a left child, then rotate that too
    if (root_mark->has_left()) {
        parent->set_right(root_mark->link[LEFT]);
    } else {
        parent->remove_right();
    }

    // Child's right child is now the parent
    root_mark->set_left(parent);

    // Update balance factors
    if (root_mark->balance == 0) {
        parent->balance = 1;
        root_mark->balance = -1;
    } else {
        parent->balance = 0;
        root_mark->balance = 0;
    }

    return root_mark;
}

/**
 * @brief Right-Left rotation around the provided parent mark
 * 
 * @param parent        Parent mark to be rotated around
 * @return bookmark*    New parent mark, after rotation
 */
bookmark* mark_tree::right_left(bookmark* parent) {

    // Store the address of the child and grandchild, 
    bookmark* child_mark = parent->link[RIGHT];
    bookmark* grandchild_mark = child_mark->link[LEFT];

    // Set the grandchild in place
    grandchild_mark->parent = parent->parent;
    if (parent->is_right()) {
        parent->parent->set_right(grandchild_mark);
    } else {
        parent->parent->set_left(grandchild_mark);
    }

    // Move the grandchild's children onto the parent and child, 
    // and then move down the parent and child to be the new
    // children of the current grandchild

    // Right Child
    if (grandchild_mark->has_right()) {
        child_mark->set_left(grandchild_mark->link[RIGHT]);
    } else {
        child_mark->remove_left();
    }
    grandchild_mark->set_right(child_mark);

    // Left Child
    if (grandchild_mark->has_left()) {
        parent->set_right(grandchild_mark->link[LEFT]);
    } else {
        parent->remove_right();
    }
    grandchild_mark->set_left(parent);
    
    // Update balance factors
    if (grandchild_mark->balance == 0) {
        parent->balance = 0;
        child_mark->balance = 0;
    } else if (grandchild_mark->balance > 0) {
        grandchild_mark->balance = 0;
        parent->balance = -1;
        child_mark->balance = 0;
    } else {
        grandchild_mark->balance = 0;
        parent->balance = 0;
        child_mark->balance = 1;
    }

    // Update balance factors
    if (grandchild_mark->balance == 0) {
        parent->balance = 0;
        child_mark->balance = 0;
    } else if (grandchild_mark->balance > 0) {
        grandchild_mark->balance = 0;
        parent->balance = -1;
        child_mark->balance = 0;
    } else {
        grandchild_mark->balance = 0;
        parent->balance = 0;
        child_mark->balance = 1;
    }

    // Return the new base mark
    return grandchild_mark;

}

/**
 * @brief Left-Right rotation around the provided parent mark
 * 
 * @param parent        Parent mark to be rotated around
 * @return bookmark*    New parent mark, after rotation
 */
bookmark* mark_tree::left_right(bookmark* parent) {
    
    // Store the address of the child and grandchild, 
    bookmark* child_mark = parent->link[LEFT];
    bookmark* grandchild_mark = child_mark->link[RIGHT];

    // Set the grandchild in place
    grandchild_mark->parent = parent->parent;
    if (parent->is_right()) {
        parent->parent->set_right(grandchild_mark);
    } else {
        parent->parent->set_left(grandchild_mark);
    }

    // Move the grandchild's children onto the parent and child, 
    // and then move down the parent and child to be the new
    // children of the current grandchild

    // Right Child
    if (grandchild_mark->has_right()) {
        parent->set_left(grandchild_mark->link[RIGHT]);
    } else {
        parent->remove_left();
    }
    grandchild_mark->set_right(parent);

    // Left Child
    if (grandchild_mark->has_left()) {
        child_mark->set_right(grandchild_mark->link[LEFT]);
    } else {
        child_mark->remove_right();
    }
    grandchild_mark->set_left(child_mark);
    
    // Update balance factors
    if (grandchild_mark->balance == 0) {
        parent->balance = 0;
        child_mark->balance = 0;
    } else if (grandchild_mark->balance > 0) {
        grandchild_mark->balance = 0;
        parent->balance = 0;
        child_mark->balance = -1;
    } else {
        grandchild_mark->balance = 0;
        parent->balance = 1;
        child_mark->balance = 0;
    }

    // Return the new base mark
    return grandchild_mark;
}

/**
 * @brief Right-Left rotation around the root node
 * 
 * @return bookmark*    New Root Node
 */
bookmark* mark_tree::right_left() {

    // Store the address of family of marks
    bookmark* parent = root_mark;
    bookmark* child_mark = parent->link[RIGHT];
    root_mark = child_mark->link[LEFT];

    // Move the grandchild's children onto the parent and child, 
    // and then move down the parent and child to be the new
    // children of the current grandchild

    // Right Child
    if (root_mark->has_right()) {
        child_mark->set_left(root_mark->link[RIGHT]);
    } else {
        child_mark->remove_left();
    }
    root_mark->set_right(child_mark);

    // Left Child
    if (root_mark->has_left()) {
        parent->set_right(root_mark->link[LEFT]);
    } else {
        parent->remove_right();
    }
    root_mark->set_left(parent);
    
    // Update balance factors
    if (root_mark->balance == 0) {
        parent->balance = 0;
        child_mark->balance = 0;
    } else if (root_mark->balance > 0) {
        root_mark->balance = 0;
        parent->balance = -1;
        child_mark->balance = 0;
    } else {
        root_mark->balance = 0;
        parent->balance = 0;
        child_mark->balance = 1;
    }

    // Return the new base mark
    return root_mark;
}

/**
 * @brief Left-Right around the root node
 * 
 * @return bookmark*    New root node
 */
bookmark* mark_tree::left_right() {
    
    // Store the address of the family of marks
    bookmark* parent = root_mark;
    bookmark* child_mark = parent->link[LEFT];
    root_mark = child_mark->link[RIGHT];

    // Move the grandchild's children onto the parent and child, 
    // and then move down the parent and child to be the new
    // children of the current grandchild

    // Right Child
    if (root_mark->has_right()) {
        parent->set_left(root_mark->link[RIGHT]);
    } else {
        parent->remove_left();
    }
    root_mark->set_right(parent);

    // Left Child
    if (root_mark->has_left()) {
        child_mark->set_right(root_mark->link[LEFT]);
    } else {
        child_mark->remove_right();
    }
    root_mark->set_left(child_mark);

    // Update balance factors
    if (root_mark->balance == 0) {
        parent->balance = 0;
        child_mark->balance = 0;
    } else if (root_mark->balance > 0) {
        root_mark->balance = 0;
        parent->balance = 0;
        child_mark->balance = -1;
    } else {
        root_mark->balance = 0;
        parent->balance = 1;
        child_mark->balance = 0;
    }

    return root_mark;
}

/* #endregion */

/* #region balance functions */

/**
 * @brief Balances the given mark according to the inputted change caused
 *        by an insertion, and balances the rest of the tree as well.
 * 
 * @param current_mark  Mark to start the balancing at
 * @param change        Needed change in the mark's balance
 */
void mark_tree::balance_insertion(bookmark* current_mark, int8_t change) {

    current_mark->balance += change;

    while (current_mark->balance != 0) {

        bool root_node = false;
        if (current_mark == root_mark) {
            root_node = true;
        }

        // If balance factor >= 2 then right tree needs rebalancing
        if (current_mark->balance >= 2) {
            // Balance right tree

            // If the child is negative (reverse of parent),
            // then a right_left rotation is needed instead
            if (current_mark->link[RIGHT]->balance < 0) {
                current_mark = (root_node ? right_left() : right_left(current_mark));
            } else {
                current_mark = (root_node ? left() : left(current_mark));
            }

            // If these fixed the balance here, then break here
            if (current_mark->balance == 0) {
                break;
            }

        // If balance factor <= -2 then left tree needs rebalancing
        } else if (current_mark->balance <= -2) {
            // Balance left tree

            // If the child is positive (reverse of parent),
            // then a left_right rotation is needed instead
            if (current_mark->link[LEFT]->balance > 0) {
                current_mark = (root_node ? left_right() : left_right(current_mark));
            } else {
                current_mark = (root_node ? right() : right(current_mark));
            }

            // If these fixed the balance here, then break here
            if (current_mark->balance == 0) {
                break;
            }
        }

        // If this is the root node, end here
        if (root_node) {
            break;
        }

        // Move to the parent, and update it's balance factor
        int8_t modifier = -1;
        if (current_mark->is_right()) {
            modifier = 1;
        }
        current_mark = current_mark->parent;
        current_mark->balance += modifier;
    }
}

/**
 * @brief Balances the given mark according to the inputted change caused
 *        by a deletion, and balances the rest of the tree as well.
 * 
 * @param current_mark  Mark to start the balancing at
 * @param change        Needed change in the mark's balance
 */
void mark_tree::balance_deletion(bookmark* current_mark, int8_t change) {

    current_mark->balance += change;

    while (current_mark->balance != 1 && current_mark->balance != -1) {

        bool root_node = false;
        if (current_mark == root_mark) {
            root_node = true;
        }

        // If balance factor >= 2 then right tree needs rebalancing
        if (current_mark->balance >= 2) {
            // Balance right tree

            // If the child is negative (reverse of parent),
            // then a right_left rotation is needed instead
            if (current_mark->link[RIGHT]->balance < 0) {
                current_mark = (root_node ? right_left() : right_left(current_mark));
            } else {
                current_mark = (root_node ? left() : left(current_mark));
            }

            // If these fixed the balance here, then break here
            if (current_mark->balance == 1 || current_mark->balance == -1) {
                break;
            }

        // If balance factor <= -2 then left tree needs rebalancing
        } else if (current_mark->balance <= -2) {
            // Balance left tree

            // If the child is positive (reverse of parent),
            // then a left_right rotation is needed instead
            if (current_mark->link[LEFT]->balance > 0) {
                current_mark = (root_node ? left_right() : left_right(current_mark));
            } else {
                current_mark = (root_node ? right() : right(current_mark));
            }

            // If these fixed the balance here, then break here
            if (current_mark->balance == 1 || current_mark->balance == -1) {
                break;
            }
        }

        // If this is the root node, end here
        if (root_node) {
            break;
        }

        // Move to the parent, and update it's balance factor
        int8_t modifier = -1;
        if (current_mark->is_left()) {
            modifier = 1;
        }
        current_mark = current_mark->parent;
        current_mark->balance += modifier;
    }
}

/* #endregion */

/* #region simple modifications */

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

    // Check if the tree hasn't been created yet
    if (current_mark == 0) {
        root_mark = new_mark;
        root_mark->flags &= ~(LEFT_CHILD | RIGHT_CHILD);
        root_mark->balance = 0;
        return;
    }

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
            if (!(current_mark->has_left())) {
                current_mark->set_left(new_mark);
                balance_change = -1;
                break;
            } else {
                current_mark = current_mark->link[LEFT];
            }
        } else {
            if (!(current_mark->has_right())) {
                current_mark->set_right(new_mark);
                balance_change = 1;
                break;
            } else {
                current_mark = current_mark->link[RIGHT];
            }
        }
    }

    // Rebalance the tree
    balance_insertion(current_mark, balance_change);
}

/**
 * @brief Removes a mark based on it's start address
 * 
 * @param target_start  Start address of the mark to be removed
 */
void mark_tree::remove(void* target_start) {

    // Find the target mark in the tree
    bookmark* target_mark = find(target_start);

    // Seperate the mark from the rest of the tree
    seperate(target_mark);
}

/**
 * @brief Removes a mark based on it's start address and size
 * 
 * @param target_start  Start address of the mark to be removed
 * @param size          Size of the mark to be removed
 */
void mark_tree::remove(void* target_start, size_t size) {

    // Find the target mark in the tree, assisted by the size
    bookmark* target_mark = find(target_start, size);
    
    // Seperate the mark from the rest of the tree
    seperate(target_mark);
}

/**
 * @brief Seperates/deletes a certain mark from the rest of the tree,
 *        keeping all other marks intact
 * 
 * @param target_mark   Mark to be deleted
 */
void mark_tree::seperate(bookmark* target_mark) {

    // If this is the root node then do something different
    if (target_mark == root_mark) {
        seperate_root();
        return;
    }

    bookmark* parent_mark = target_mark->parent;
    int8_t balance_change = (target_mark->is_right() ? -1 : 1);

    // Check if the mark has children
    bool right_child = target_mark->has_right();
    bool left_child = target_mark->has_left();

    if (right_child && left_child) {

        // It has children on both sides (longest case)

        // Seperate right child
        mark_tree right (target_mark->link[RIGHT], this->size_sorted, true);
        target_mark->remove_right();

        // Move left child into target's place
        if (target_mark->is_right()) {
            parent_mark->set_right(target_mark->link[LEFT]);
        } else {
            parent_mark->set_left(target_mark->link[LEFT]);
        }

        // Rebalance the tree
        balance_deletion(parent_mark, balance_change);

        // Put the right child back on (will handle balancing itself)
        *this = tree_union(*this, right);

    } else {

        if (right_child) {

            // Only a right child
            if (target_mark->is_right()) {
                parent_mark->set_right(target_mark->link[RIGHT]);
            } else {
                parent_mark->set_left(target_mark->link[RIGHT]);
            }

        } else if (left_child) {

            // Only a left child
            if (target_mark->is_right()) {
                parent_mark->set_right(target_mark->link[LEFT]);
            } else {
                parent_mark->set_left(target_mark->link[LEFT]);
            }

        } else {

            // No children
            if (target_mark->is_right()) {
            parent_mark->remove_right();
            } else {
                parent_mark->remove_left();
            }

        }

        // Rebalance the tree
        balance_deletion(parent_mark, balance_change);
    }
}

/**
 * @brief Removes the root mark from the tree
 * 
 */
void mark_tree::seperate_root() {
    
    bookmark* target_mark = root_mark;

    // Check if the mark has children
    bool right_child = target_mark->has_right();
    bool left_child = target_mark->has_left();

    if (right_child && left_child) {

        // It has children on both sides (longest case)

        // Seperate right child
        mark_tree right (target_mark->link[RIGHT], this->size_sorted, true);
        target_mark->remove_right();

        // Move left child into target's place
        root_mark = target_mark->link[LEFT];

        // Put the right child back on (will handle balancing itself)
        *this = tree_union(*this, right);

    } else {

        if (right_child) {

            // Only a right child
            root_mark = target_mark->link[RIGHT];

        } else if (left_child) {

            // Only a left child
            root_mark = target_mark->link[LEFT];

        } else {

            // No children
            root_mark = 0;

        }

        // No rebalancing needs to be done
    }
}

/* #endregion */

/* #endregion */

/* #region set operations */

/**
 * @brief Joins a smaller right tree onto a larger left one
 * 
 * @param left          Left tree
 * @param left_height   Height of the left tree
 * @param joiner        Joiner mark which will be added to the combined tree
 * @param right         Right tree
 * @param right_height  Height of the right tree
 * @return mark_tree    New combined tree
 */
mark_tree join_right(mark_tree left, unsigned int left_height, bookmark* joiner,
                     mark_tree right, unsigned int right_height) {

    bookmark* insertion_point = left.root_mark;

    // Determine the correct insertion point
    while (left_height > right_height) {
        insertion_point = insertion_point->link[RIGHT];
        left_height--;
    }

    insertion_point->parent->set_right(joiner);
    joiner->set_left(insertion_point);
    joiner->set_right(right.root_mark);
    joiner->balance = 0;

    left.balance_insertion(joiner->parent, 1);

    return left;
};

/**
 * @brief Joins a smaller left tree onto a larger right one
 * 
 * @param left          Left tree
 * @param left_height   Height of the left tree
 * @param joiner        Joiner mark which will be added to the combined tree
 * @param right         Right tree
 * @param right_height  Height of the right tree
 * @return mark_tree    New combined tree
 */
mark_tree join_left(mark_tree left, unsigned int left_height, bookmark* joiner,
                     mark_tree right, unsigned int right_height) {

    bookmark* insertion_point = right.root_mark;

    // Determine the correct insertion point
    while (left_height < right_height) {
        insertion_point = insertion_point->link[LEFT];
        right_height--;
    }

    insertion_point->parent->set_left(joiner);
    joiner->set_right(insertion_point);
    joiner->set_left(left.root_mark);
    joiner->balance = 0;

    right.balance_insertion(joiner->parent, -1);

    return right;
};

/**
 * @brief Joins a left tree to a right tree, with an additional joiner
 *        bookmark. For all elements of both trees, left < joiner < right
 *        must be true. It is assumed that the trees have the same
 *        style of sorting.
 * 
 * @param left          Left tree
 * @param joiner        New mark which will be used to combine the trees
 * @param right         Right tree
 * @return mark_tree    Combined tree made of (left + joiner + right) 
 */
mark_tree join(mark_tree left, bookmark* joiner, mark_tree right) {

    unsigned int left_height = left.get_height();
    unsigned int right_height = right.get_height();

    if (left_height > (right_height + 1)) {
        return join_right(left, left_height, joiner, right, right_height);
    } else if (right_height > (left_height + 1)) {
        return join_left(left, left_height, joiner, right, right_height);
    } else {

        // These trees can just be joined by creating a new tree,
        // with the joiner as the root node, and the two trees
        // as it's child subtrees.
        mark_tree new_tree (joiner, right.size_sorted);
        if (left_height > 0) 
            new_tree.root_mark->set_left(left.root_mark);
        if (right_height > 0)
            new_tree.root_mark->set_right(right.root_mark);

        // Fix the balance of the joiner
        joiner->balance = (right_height - left_height);

        return new_tree;
    }
};

/**
 * @brief Splits the target tree into two new trees, left_tree and
 *        right_tree, based on what's smaller or larger than the 
 *        key mark, sorted by size
 * 
 * @param left_tree     Blank &tree to be filled with marks smaller than key
 * @param right_tree    Blank &tree to be filled with marks larger than key
 * @param target_tree   Tree to be split into left_tree and right_tree
 * @param key           Key mark to determine how the target is split
 */
void split_size(mark_tree& left_tree, mark_tree& right_tree, 
           mark_tree target_tree, bookmark* key) {

    // Check if the tree would be split down the middle
    if (key->size == target_tree.root_mark->size) {

        if (target_tree.root_mark->has_left()) 
            left_tree.root_mark = target_tree.root_mark->link[LEFT];
        if (target_tree.root_mark->has_right()) 
            right_tree.root_mark = target_tree.root_mark->link[RIGHT];

    } else if (key->size < target_tree.root_mark->size) {

        // Go down the left side (if it exists)
        if (target_tree.root_mark->has_left()) {
            mark_tree right_derivative (false);
            bookmark* derivative_key = target_tree.root_mark;

            mark_tree right_whole (target_tree.root_mark->link[RIGHT], 
                                   true, true);

            target_tree.root_mark = target_tree.root_mark->link[LEFT];
            split_size(left_tree, right_derivative, target_tree, key);

            right_tree = join(right_derivative, derivative_key, right_whole);
        } else {
            right_tree.insert(target_tree.root_mark);
        }

    } else {

        // Go down the right side (if it exists)
        if (target_tree.root_mark->has_right()) {
            mark_tree left_derivative (false);
            bookmark* derivative_key = target_tree.root_mark;

            mark_tree left_whole (target_tree.root_mark->link[RIGHT], 
                                   true, true);
            
            target_tree.root_mark = target_tree.root_mark->link[RIGHT];
            split_size(left_derivative, right_tree, target_tree, key);

            left_tree = join(left_whole, derivative_key, left_derivative);
        } else {
            left_tree.insert(target_tree.root_mark);
        }

    }
}

/**
 * @brief Splits the target tree into two new trees, left_tree and
 *        right_tree, based on what's smaller or larger than the 
 *        key mark, sorted by start address
 * 
 * @param left_tree     Blank &tree to be filled with marks smaller than key
 * @param right_tree    Blank &tree to be filled with marks larger than key
 * @param target_tree   Tree to be split into left_tree and right_tree
 * @param key           Key mark to determine how the target is split
 */
void split_addr(mark_tree& left_tree, mark_tree& right_tree, 
           mark_tree target_tree, bookmark* key) {

    // Check if the tree would be split down the middle
    if (key->start == target_tree.root_mark->start) {

        if (target_tree.root_mark->has_left()) 
            left_tree.root_mark = target_tree.root_mark->link[LEFT];
        if (target_tree.root_mark->has_right()) 
            right_tree.root_mark = target_tree.root_mark->link[RIGHT];

    } else if (key->start < target_tree.root_mark->start) {

        // Go down the left side (if it exists)
        if (target_tree.root_mark->has_left()) {
            mark_tree right_derivative (false);
            bookmark* derivative_key = target_tree.root_mark;

            mark_tree right_whole (target_tree.root_mark->link[RIGHT], 
                                   false, true);

            target_tree.root_mark = target_tree.root_mark->link[LEFT];
            split_addr(left_tree, right_derivative, target_tree, key);

            right_tree = join(right_derivative, derivative_key, right_whole);
        } else {
            right_tree.insert(target_tree.root_mark);
        }

    } else {

        // Go down the right side (if it exists)
        if (target_tree.root_mark->has_right()) {
            mark_tree left_derivative (false);
            bookmark* derivative_key = target_tree.root_mark;

            mark_tree left_whole (target_tree.root_mark->link[LEFT], 
                                   false, true);
            
            target_tree.root_mark = target_tree.root_mark->link[RIGHT];
            split_addr(left_derivative, right_tree, target_tree, key);

            left_tree = join(left_whole, derivative_key, left_derivative);
        } else {
            left_tree.insert(target_tree.root_mark);
        }

    }
}

/**
 * @brief Splits the target tree into two new trees, left_tree and
 *        right_tree, based on what's smaller or larger than the 
 *        key mark.
 * 
 * @param left_tree     Blank &tree to be filled with marks smaller than key
 * @param right_tree    Blank &tree to be filled with marks larger than key
 * @param target_tree   Tree to be split into left_tree and right_tree
 * @param key           Key mark to determine how the target is split
 */
void split(mark_tree& left_tree, mark_tree& right_tree, 
           mark_tree target_tree, bookmark* key) {

    // Refer to the correct splitting function
    if (target_tree.size_sorted) {
        left_tree.size_sorted = true;
        right_tree.size_sorted = true;
        split_size(left_tree, right_tree, target_tree, key);
    } else {
        left_tree.size_sorted = false;
        right_tree.size_sorted = false;
        split_addr(left_tree, right_tree, target_tree, key);
    }
};

/**
 * @brief Combines two trees
 * 
 * @param tree_one      First tree
 * @param tree_two      Second tree
 * @return mark_tree    Combined tree made from tree_one and tree_two
 */
mark_tree tree_union(mark_tree tree_one, mark_tree tree_two) {

    // Get the heights of both trees
    unsigned int one_height = tree_one.get_height();
    unsigned int two_height = tree_two.get_height();

    // Check to see if either tree is empty (or close to it)
    if (tree_one.root_mark == 0) {
        return tree_two;
    } else if (one_height <= 1) {
        tree_two.insert(tree_one.root_mark);
        return tree_two;
    }

    if (tree_two.root_mark == 0) {
        return tree_one;
    } else if (two_height <= 1) {
        tree_one.insert(tree_two.root_mark);
        return tree_one;
    }

    // Split tree two into two parts
    mark_tree left_derivative;
    mark_tree right_derivative;
    split(left_derivative, right_derivative, tree_two, tree_one.root_mark);

    // Get the two sides of tree one
    mark_tree one_left (tree_one.root_mark->link[LEFT], tree_one.size_sorted, true);
    mark_tree one_right (tree_one.root_mark->link[RIGHT], tree_one.size_sorted, true);

    // Combine each pair of sides with eachother
    left_derivative = tree_union(one_left, left_derivative);
    right_derivative = tree_union(one_right, right_derivative);

    // Join the two combined sides with eachother, and return the result
    return join(left_derivative, tree_one.root_mark, right_derivative);
};

/* #endregion */

}
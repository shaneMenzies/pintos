#ifndef PINT_AVL_TREE_H
#define PINT_AVL_TREE_H

#include "common.h"
#include "operators.h"

namespace std_k {

enum link { RIGHT = 1, LEFT = 0 };

template<class T> struct avl_node {

    avl_node<T>* link[2] = {nullptr, nullptr};

    avl_node<T>* parent         = nullptr;
    char         balance        = 0;
    bool         is_right_child = false;

    T value;

    avl_node() {}
    avl_node(T value)
        : value(value) {}

    inline bool has_left() { return (link[LEFT] != nullptr); }
    inline bool has_right() { return (link[RIGHT] != nullptr); }
    inline bool has_both() { return (has_left() && has_right()); }
    inline bool has_any() { return (has_left() || has_right()); }

    inline bool is_left() { return !is_right_child; }
    inline bool is_right() { return is_right_child; }

    inline void set_right(avl_node<T>* new_child) {
        link[RIGHT] = new_child;
        if (new_child != nullptr) {
            new_child->parent         = this;
            new_child->is_right_child = true;
        }
    }
    inline void set_left(avl_node<T>* new_child) {
        link[LEFT] = new_child;
        if (new_child != nullptr) {
            new_child->parent         = this;
            new_child->is_right_child = false;
        }
    };

    inline avl_node<T>* get_right() { return link[RIGHT]; }
    inline avl_node<T>* get_left() { return link[LEFT]; }
    inline avl_node<T>* get_parent() { return parent; }

    inline void remove_right() { link[RIGHT] = nullptr; }
    inline void remove_left() { link[LEFT] = nullptr; }
    inline void remove_any() {
        link[RIGHT] = nullptr;
        link[LEFT]  = nullptr;
    }
};

template<class T, class comparison = less<T>> class avl_tree {

  private:
    // Simple rotations around a certain parent node
    avl_node<T>* right(avl_node<T>*);
    avl_node<T>* left(avl_node<T>*);

    // Simple rotations around the root node
    avl_node<T>* right();
    avl_node<T>* left();

    // Double rotations around a certain parent node
    avl_node<T>* right_left(avl_node<T>*);
    avl_node<T>* left_right(avl_node<T>*);

    // Double rotations around the root node
    avl_node<T>* right_left();
    avl_node<T>* left_right();

  public:
    using node             = avl_node<T>;
    avl_node<T>* root_node = 0;

    avl_tree() {}
    avl_tree(avl_node<T>* root)
        : root_node(root) {}

    unsigned int get_height();

    // Search functions
    template<class end_condition = std_k::equal_to<T>>
    avl_node<T>* find(T target);
    template<class end_condition = std_k::equal_to<T>>
    avl_node<T>* find(T target_value, int index);

    // Simple modifications
    void insert(avl_node<T>* new_node);
    void remove(T target);

    // Function that seperates a certain T from the
    // rest of the tree
    void seperate(avl_node<T>* target);
    void seperate_root();

    // Functions that balances the tree after a change
    void balance_insertion(avl_node<T>* target, int8_t change);
    void balance_deletion(avl_node<T>* target, int8_t change);
};

/* #region avl_tree<T> functions */

/**
 * @brief Calculates the height of this tree, and returns it
 *
 * @return unsigned int     Current height of this tree, at it's tallest point
 */
template<class T, class comparison>
unsigned int avl_tree<T, comparison>::get_height() {

    avl_node<T>* current_node = root_node;

    // If tree not created yet, return 0
    if (current_node == 0) { return 0; }

    unsigned int total_height = 0;

    while (1) {
        total_height++;

        // Only break out of this loop once there are no more children
        if (!current_node->has_left() && !current_node->has_right()) { break; }

        // Move based on balance of this node
        if (current_node->balance < 0) {
            current_node = current_node->link[LEFT];
        } else {
            current_node = current_node->link[RIGHT];
        }

        if (current_node == nullptr) break;
    }

    return total_height;
}

/**
 * @brief Finds the node representing a certain value
 *
 * @param target_value  value for the node to be found
 * @return avl_node<T>*    Pointer to the node with the requested address,
 *                      or a 0 if no node could be found
 */
template<class T, class comparison>
template<class end_condition>
avl_node<T>* avl_tree<T, comparison>::find(T target_value) {

    avl_node<T>* current_node = root_node;

    while (!(end_condition {}(current_node->value, target_value))) {

        // Move to one of the child nodes
        if (comparison()(current_node->value, target_value)
            && current_node->has_right()) {
            current_node = current_node->link[RIGHT];
        } else if (current_node->has_left()) {
            current_node = current_node->link[LEFT];
        } else {
            return 0;
        }
    }

    return current_node;
}

/**
 * @brief Finds the node representing a certain value
 *
 * @param target_value  value for the node to be found
 * @param index         index in group of matching nodes
 * @return avl_node<T>*    Pointer to the node with the requested address,
 *                      or a 0 if no node could be found
 */
template<class T, class comparison>
template<class end_condition>
avl_node<T>* avl_tree<T, comparison>::find(T target_value, int index) {

    avl_node<T>* current_node = root_node;

    while ((!(end_condition {}(current_node->value, target_value)))
           && (--index > 0)) {

        // Move to one of the child nodes
        if (comparison {}(current_node->value, target_value)
            && current_node->has_right()) {
            current_node = current_node->link[RIGHT];
        } else if (current_node->has_left()) {
            current_node = current_node->link[LEFT];
        } else {
            return 0;
        }
    }

    return current_node;
}

/* #region tree rotations */

/**
 * @brief Right-Right rotation around the provided parent node
 *
 * @param parent        Parent node to be rotated around
 * @return avl_node<T>*    New parent node, after rotation
 */
template<class T, class comparison>
avl_node<T>* avl_tree<T, comparison>::right(avl_node<T>* parent) {

    // Store the addresses of the related nodes,
    // and then rotate the child into its place.
    avl_node<T>* grandparent_node = parent->parent;
    avl_node<T>* child_node       = parent->link[LEFT];

    if (parent->is_right()) {
        grandparent_node->set_right(child_node);
    } else {
        grandparent_node->set_left(child_node);
    }

    // If the child itself has a right child, then rotate that too
    if (child_node->has_right()) {
        parent->set_left(child_node->link[RIGHT]);
    } else {
        parent->remove_left();
    }

    // Child's right child is now the parent
    child_node->set_right(parent);

    // Update balance factors
    if (child_node->balance == 0) {
        parent->balance     = -1;
        child_node->balance = 1;
    } else {
        parent->balance     = 0;
        child_node->balance = 0;
    }

    // Return the new base node
    return child_node;
}

/**
 * @brief Left-Left rotation around the provided parent node
 *
 * @param parent        Parent node to be rotated around
 * @return avl_node<T>*    New parent node, after rotation
 */
template<class T, class comparison>
avl_node<T>* avl_tree<T, comparison>::left(avl_node<T>* parent) {

    // Store the addresses of the related nodes,
    // and then rotate the child into its place.
    avl_node<T>* grandparent_node = parent->parent;
    avl_node<T>* child_node       = parent->link[RIGHT];

    if (parent->is_right()) {
        grandparent_node->set_right(child_node);
    } else {
        grandparent_node->set_left(child_node);
    }

    // If the child itself has a left child, then rotate that too
    if (child_node->has_left()) {
        parent->set_right(child_node->link[LEFT]);
    } else {
        parent->remove_right();
    }

    // Child's left child is now the parent
    child_node->set_left(parent);

    // Update balance factors
    if (child_node->balance == 0) {
        parent->balance     = 1;
        child_node->balance = -1;
    } else {
        parent->balance     = 0;
        child_node->balance = 0;
    }

    // Return the new base node
    return child_node;
}

/**
 * @brief Right-Right rotation around the root node
 *
 * @return avl_node<T>*    New Root Node
 */
template<class T, class comparison>
avl_node<T>* avl_tree<T, comparison>::right() {

    // Store the addresses of the related nodes,
    // and then rotate the child into its place.
    avl_node<T>* parent = root_node;
    root_node           = parent->link[LEFT];

    // If the child itself has a right child, then rotate that too
    if (root_node->has_right()) {
        parent->set_left(root_node->link[RIGHT]);
    } else {
        parent->remove_left();
    }

    // Child's right child is now the parent
    root_node->set_right(parent);

    // Update balance factors
    if (root_node->balance == 0) {
        parent->balance    = -1;
        root_node->balance = 1;
    } else {
        parent->balance    = 0;
        root_node->balance = 0;
    }

    // Return the new base node
    return root_node;
}

/**
 * @brief Left-Left rotation around the root node
 *
 * @return avl_node<T>*    New Root Node
 */
template<class T, class comparison>
avl_node<T>* avl_tree<T, comparison>::left() {

    // Store the addresses of the related nodes,
    // and then rotate the child into its place.
    avl_node<T>* parent = root_node;
    root_node           = parent->link[RIGHT];

    // If the child itself has a left child, then rotate that too
    if (root_node->has_left()) {
        parent->set_right(root_node->link[LEFT]);
    } else {
        parent->remove_right();
    }

    // Child's right child is now the parent
    root_node->set_left(parent);

    // Update balance factors
    if (root_node->balance == 0) {
        parent->balance    = 1;
        root_node->balance = -1;
    } else {
        parent->balance    = 0;
        root_node->balance = 0;
    }

    return root_node;
}

/**
 * @brief Right-Left rotation around the provided parent node
 *
 * @param parent        Parent node to be rotated around
 * @return avl_node<T>*    New parent node, after rotation
 */
template<class T, class comparison>
avl_node<T>* avl_tree<T, comparison>::right_left(avl_node<T>* parent) {

    // Store the address of the child and grandchild,
    avl_node<T>* child_node      = parent->link[RIGHT];
    avl_node<T>* grandchild_node = child_node->link[LEFT];

    // Set the grandchild in place
    grandchild_node->parent = parent->parent;
    if (parent->is_right()) {
        parent->parent->set_right(grandchild_node);
    } else {
        parent->parent->set_left(grandchild_node);
    }

    // Move the grandchild's children onto the parent and child,
    // and then move down the parent and child to be the new
    // children of the current grandchild

    // Right Child
    if (grandchild_node->has_right()) {
        child_node->set_left(grandchild_node->link[RIGHT]);
    } else {
        child_node->remove_left();
    }
    grandchild_node->set_right(child_node);

    // Left Child
    if (grandchild_node->has_left()) {
        parent->set_right(grandchild_node->link[LEFT]);
    } else {
        parent->remove_right();
    }
    grandchild_node->set_left(parent);

    // Update balance factors
    if (grandchild_node->balance == 0) {
        parent->balance     = 0;
        child_node->balance = 0;
    } else if (grandchild_node->balance > 0) {
        grandchild_node->balance = 0;
        parent->balance          = -1;
        child_node->balance      = 0;
    } else {
        grandchild_node->balance = 0;
        parent->balance          = 0;
        child_node->balance      = 1;
    }

    // Return the new base node
    return grandchild_node;
}

/**
 * @brief Left-Right rotation around the provided parent node
 *
 * @param parent        Parent node to be rotated around
 * @return avl_node<T>*    New parent node, after rotation
 */
template<class T, class comparison>
avl_node<T>* avl_tree<T, comparison>::left_right(avl_node<T>* parent) {

    // Store the address of the child and grandchild,
    avl_node<T>* child_node      = parent->link[LEFT];
    avl_node<T>* grandchild_node = child_node->link[RIGHT];

    // Set the grandchild in place
    grandchild_node->parent = parent->parent;
    if (parent->is_right()) {
        parent->parent->set_right(grandchild_node);
    } else {
        parent->parent->set_left(grandchild_node);
    }

    // Move the grandchild's children onto the parent and child,
    // and then move down the parent and child to be the new
    // children of the current grandchild

    // Right Child
    if (grandchild_node->has_right()) {
        parent->set_left(grandchild_node->link[RIGHT]);
    } else {
        parent->remove_left();
    }
    grandchild_node->set_right(parent);

    // Left Child
    if (grandchild_node->has_left()) {
        child_node->set_right(grandchild_node->link[LEFT]);
    } else {
        child_node->remove_right();
    }
    grandchild_node->set_left(child_node);

    // Update balance factors
    if (grandchild_node->balance == 0) {
        parent->balance     = 0;
        child_node->balance = 0;
    } else if (grandchild_node->balance > 0) {
        grandchild_node->balance = 0;
        parent->balance          = 0;
        child_node->balance      = -1;
    } else {
        grandchild_node->balance = 0;
        parent->balance          = 1;
        child_node->balance      = 0;
    }

    // Return the new base node
    return grandchild_node;
}

/**
 * @brief Right-Left rotation around the root node
 *
 * @return avl_node<T>*    New Root Node
 */
template<class T, class comparison>
avl_node<T>* avl_tree<T, comparison>::right_left() {

    // Store the address of family of nodes
    avl_node<T>* parent     = root_node;
    avl_node<T>* child_node = parent->link[RIGHT];

    root_node = child_node->link[LEFT];

    // Move the grandchild's children onto the parent and child,
    // and then move down the parent and child to be the new
    // children of the current grandchild

    // Right Child
    if (root_node->has_right()) {
        child_node->set_left(root_node->link[RIGHT]);
    } else {
        child_node->remove_left();
    }
    root_node->set_right(child_node);

    // Left Child
    if (root_node->has_left()) {
        parent->set_right(root_node->link[LEFT]);
    } else {
        parent->remove_right();
    }
    root_node->set_left(parent);

    // Update balance factors
    if (root_node->balance == 0) {
        parent->balance     = 0;
        child_node->balance = 0;
    } else if (root_node->balance > 0) {
        root_node->balance  = 0;
        parent->balance     = -1;
        child_node->balance = 0;
    } else {
        root_node->balance  = 0;
        parent->balance     = 0;
        child_node->balance = 1;
    }

    // Return the new base node
    return root_node;
}

/**
 * @brief Left-Right around the root node
 *
 * @return avl_node<T>*    New root node
 */
template<class T, class comparison>
avl_node<T>* avl_tree<T, comparison>::left_right() {

    // Store the address of the family of nodes
    avl_node<T>* parent     = root_node;
    avl_node<T>* child_node = parent->link[LEFT];

    root_node = child_node->link[RIGHT];

    // Move the grandchild's children onto the parent and child,
    // and then move down the parent and child to be the new
    // children of the current grandchild

    // Right Child
    if (root_node->has_right()) {
        parent->set_left(root_node->link[RIGHT]);
    } else {
        parent->remove_left();
    }
    root_node->set_right(parent);

    // Left Child
    if (root_node->has_left()) {
        child_node->set_right(root_node->link[LEFT]);
    } else {
        child_node->remove_right();
    }
    root_node->set_left(child_node);

    // Update balance factors
    if (root_node->balance == 0) {
        parent->balance     = 0;
        child_node->balance = 0;
    } else if (root_node->balance > 0) {
        root_node->balance  = 0;
        parent->balance     = 0;
        child_node->balance = -1;
    } else {
        root_node->balance  = 0;
        parent->balance     = 1;
        child_node->balance = 0;
    }

    return root_node;
}

/* #endregion */

/* #region balance functions */

/**
 * @brief Balances the given node according to the inputted change caused
 *        by an insertion, and balances the rest of the tree as well.
 *
 * @param current_node  node to start the balancing at
 * @param change        Needed change in the node's balance
 */
template<class T, class comparison>
void avl_tree<T, comparison>::balance_insertion(avl_node<T>* current_node,
                                                int8_t       change) {

    current_node->balance += change;

    while (current_node->balance != 0) {

        bool at_root_node = false;
        if (current_node == root_node) { at_root_node = true; }

        // If balance factor >= 2 then right tree needs rebalancing
        if (current_node->balance >= 2) {
            // Balance right tree

            // If the child is negative (reverse of parent),
            // then a right_left rotation is needed instead
            if (current_node->link[RIGHT]->balance < 0) {
                current_node
                    = (at_root_node ? right_left() : right_left(current_node));
            } else {
                current_node = (at_root_node ? left() : left(current_node));
            }

            // If these fixed the balance here, then break here
            if (current_node->balance == 0) { break; }

            // If balance factor <= -2 then left tree needs rebalancing
        } else if (current_node->balance <= -2) {
            // Balance left tree

            // If the child is positive (reverse of parent),
            // then a left_right rotation is needed instead
            if (current_node->link[LEFT]->balance > 0) {
                current_node
                    = (at_root_node ? left_right() : left_right(current_node));
            } else {
                current_node = (at_root_node ? right() : right(current_node));
            }

            // If these fixed the balance here, then break here
            if (current_node->balance == 0) { break; }
        }

        // If this is the root node, end here
        if (at_root_node) { break; }

        // Move to the parent, and update it's balance factor
        int8_t modifier = -1;
        if (current_node->is_right()) { modifier = 1; }
        current_node = current_node->parent;
        current_node->balance += modifier;
    }
}

/**
 * @brief Balances the given node according to the inputted change caused
 *        by a deletion, and balances the rest of the tree as well.
 *
 * @param current_node  node to start the balancing at
 * @param change        Needed change in the node's balance
 */
template<class T, class comparison>
void avl_tree<T, comparison>::balance_deletion(avl_node<T>* current_node,
                                               int8_t       change) {

    current_node->balance += change;

    while (current_node->balance != 1 && current_node->balance != -1) {

        bool at_root_node = false;
        if (current_node == root_node) { at_root_node = true; }

        // If balance factor >= 2 then right tree needs rebalancing
        if (current_node->balance >= 2) {
            // Balance right tree

            // If the child is negative (reverse of parent),
            // then a right_left rotation is needed instead
            if (current_node->link[RIGHT]->balance < 0) {
                current_node
                    = (at_root_node ? right_left() : right_left(current_node));
            } else {
                current_node = (at_root_node ? left() : left(current_node));
            }

            // If these fixed the balance here, then break here
            if (current_node->balance == 1 || current_node->balance == -1) {
                break;
            }

            // If balance factor <= -2 then left tree needs rebalancing
        } else if (current_node->balance <= -2) {
            // Balance left tree

            // If the child is positive (reverse of parent),
            // then a left_right rotation is needed instead
            if (current_node->link[LEFT]->balance > 0) {
                current_node
                    = (at_root_node ? left_right() : left_right(current_node));
            } else {
                current_node = (at_root_node ? right() : right(current_node));
            }

            // If these fixed the balance here, then break here
            if (current_node->balance == 1 || current_node->balance == -1) {
                break;
            }
        }

        // If this is the root node, end here
        if (at_root_node) { break; }

        // Move to the parent, and update it's balance factor
        int8_t modifier = -1;
        if (current_node->is_left()) { modifier = 1; }
        current_node = current_node->parent;
        current_node->balance += modifier;
    }
}

/* #endregion */

/* #region simple modifications */

/**
 * @brief Places a new node into this tree, sorting it into the correct position
 *        based on either size or address. This function also does rebalance
 *        the tree after placing the new node.
 *              - Note: In cases of sorting by size, where the size of the new
 *                      node is the same as another one, the new node will
 *                      always get placed on the RIGHT side of the old one
 *
 * @param new_node New node to be placed in the tree
 */
template<class T, class comparison>
void avl_tree<T, comparison>::insert(avl_node<T>* new_node) {

    new_node->balance = 0;
    new_node->link[0] = 0;
    new_node->link[1] = 0;
    new_node->parent  = 0;

    avl_node<T>* current_node = root_node;

    // Check if the tree hasn't been created yet
    if (current_node == 0) {
        root_node                 = new_node;
        root_node->is_right_child = false;
        root_node->balance        = 0;
        return;
    }

    int8_t balance_change;

    // Search through the tree to find the correct spot for the new node
    while (1) {

        // Determine direction from current node
        link direction = comparison {}(current_node->value, new_node->value)
                             ? RIGHT
                             : LEFT;

        // If the direction doesn't have a child then put the new node there
        // and break, otherwise just travel down that path
        if (direction == LEFT) {
            if (!(current_node->has_left())) {
                current_node->set_left(new_node);
                balance_change = -1;
                break;
            } else {
                current_node = current_node->link[LEFT];
            }
        } else {
            if (!(current_node->has_right())) {
                current_node->set_right(new_node);
                balance_change = 1;
                break;
            } else {
                current_node = current_node->link[RIGHT];
            }
        }
    }

    // Rebalance the tree
    balance_insertion(current_node, balance_change);
}

/**
 * @brief Removes a node from the tree
 *
 * @param target    Value of the target
 */
template<class T, class comparison>
void avl_tree<T, comparison>::remove(T target) {

    // Find the target node in the tree, assisted by the size
    avl_node<T>* target_node = find(target);

    // Seperate the node from the rest of the tree
    if (target_node != nullptr) seperate(target_node);
}

/**
 * @brief Seperates/deletes a certain node from the rest of the tree,
 *        keeping all other nodes intact
 *
 * @param target_node   node to be deleted
 */
template<class T, class comparison>
void avl_tree<T, comparison>::seperate(avl_node<T>* target_node) {

    // If this is the root node then do something different
    if (target_node == root_node) {
        seperate_root();
        return;
    }

    avl_node<T>* parent_node    = target_node->parent;
    int8_t       balance_change = (target_node->is_right() ? -1 : 1);

    // Check if the node has children
    bool right_child = target_node->has_right();
    bool left_child  = target_node->has_left();

    if (right_child && left_child) {

        // It has children on both sides (longest case)

        // Find which node should replace this one (smallest in right child)
        avl_node<T>* successor           = target_node->get_right();
        bool         immediate_successor = !successor->has_left();
        while (successor->has_left()) successor = successor->get_left();

        if (!immediate_successor) {
            // Need to remove successor from its position first
            avl_node<T>* s_parent = successor->parent;
            successor->parent->set_left(successor->get_right());
            successor->set_right(target_node->get_right());

            // Swap target with successor
            if (target_node->is_right())
                parent_node->set_right(successor);
            else
                parent_node->set_left(successor);

            successor->set_left(target_node->get_left());
            successor->balance = target_node->balance;

            balance_deletion(s_parent, 1);
        } else {

            // Swap target with successor
            if (target_node->is_right())
                parent_node->set_right(successor);
            else
                parent_node->set_left(successor);

            successor->set_left(target_node->get_left());
            successor->balance = target_node->balance;

            balance_deletion(successor, -1);
        }

    } else {

        if (right_child) {

            // Only a right child
            if (target_node->is_right()) {
                parent_node->set_right(target_node->link[RIGHT]);
            } else {
                parent_node->set_left(target_node->link[RIGHT]);
            }

        } else if (left_child) {

            // Only a left child
            if (target_node->is_right()) {
                parent_node->set_right(target_node->link[LEFT]);
            } else {
                parent_node->set_left(target_node->link[LEFT]);
            }

        } else {

            // No children
            if (target_node->is_right()) {
                parent_node->remove_right();
            } else {
                parent_node->remove_left();
            }
        }

        // Rebalance the tree
        balance_deletion(parent_node, balance_change);
    }
}

/**
 * @brief Removes the root node from the tree
 *
 */
template<class T, class comparison>
void avl_tree<T, comparison>::seperate_root() {

    avl_node<T>* target_node = root_node;

    // Check if the node has children
    bool right_child = target_node->has_right();
    bool left_child  = target_node->has_left();

    if (right_child && left_child) {

        // It has children on both sides (longest case)

        // Find which node should replace this one (smallest in right child)
        avl_node<T>* successor           = target_node->get_right();
        bool         immediate_successor = !successor->has_left();
        while (successor->has_left()) successor = successor->get_left();

        if (!immediate_successor) {
            // Need to remove successor from its position first
            successor->parent->set_left(successor->get_right());
            balance_deletion(successor->parent, 1);

            successor->set_right(target_node->get_right());

            // Swap target with successor
            root_node = successor;

            successor->set_left(target_node->get_left());
            successor->balance = target_node->balance;
        } else {

            // Swap target with successor
            root_node = successor;

            successor->set_left(target_node->get_left());
            successor->balance = target_node->balance;

            balance_deletion(successor, -1);
        }

    } else {

        if (right_child) {

            // Only a right child
            root_node = target_node->link[RIGHT];

        } else if (left_child) {

            // Only a left child
            root_node = target_node->link[LEFT];

        } else {

            // No children
            root_node = 0;
        }

        // No rebalancing needs to be done
    }
}

/* #endregion */

/* #endregion */

} // namespace std_k

#endif

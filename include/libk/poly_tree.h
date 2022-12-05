#ifndef PINTOS_POLY_TREE_H
#define PINTOS_POLY_TREE_H

#include "common.h"
#include "operators.h"
#include "sorting.h"
#include "vector.h"

namespace std_k {

template<typename T, class compare = std_k::less<T>> struct poly_node {
    T                              value;
    poly_node<T, compare>*         parent   = nullptr;
    vector<poly_node<T, compare>*> children = (0);

    poly_node(T value)
        : value(value) {}
    poly_node(T value, poly_node<T, compare>* initial_children[],
              size_t initial_size)
        : value(value)
        , children(initial_children, initial_size, initial_size) {}

    operator T() { return value; }

    poly_node<T, compare>* child(size_t index) { return children[index]; }

    poly_node<T, compare>* operator[](size_t index) { return children[index]; }

    void add_child(poly_node<T, compare>* new_child) {
        int index = std_k::binary_find_new_index_pointer(
            children.data(), children.size(), new_child);
        if (index >= 0) { children.insert(index, new_child); }
    }

    void remove_child(size_t index) { children.erase(index); }
    void remove_child(poly_node<T, compare>* target) {
        int index
            = binary_match_pointer(children.data(), children.size(), target);
        if (index >= 0) { children.erase(index); }
    }
};

template<typename T, class compare = std_k::less<T>> class poly_tree {
  public:
    using node = poly_node<T, compare>;
    node* root;

    poly_tree(node* root)
        : root(root) {}

    int* path_to(node* target, int* buffer) {
        int   depth   = 0;
        node* current = target;

        while (current != root) {
            // Move to parent
            node* last = current;
            current    = current->parent;

            // Find index
            int i = std_k::binary_match_pointer<T>(current->children.data(),
                                                   current->children.size(),
                                                   last->value);
            buffer[depth] = i;
            depth++;
        }

        // Need to reverse current buffer to be path from root
        for (int i = 0; i < depth; i++) {
            int temp                = buffer[i];
            buffer[i]               = buffer[(depth - i - 1)];
            buffer[(depth - i - 1)] = temp;
        }

        // Path is terminated with -1
        buffer[depth] = -1;

        return buffer;
    }

    node* node_at(int* path) {
        node* current = root;

        int i = 0;
        while (path[i] >= 0 && current != nullptr) {
            current = current->children[path[i]];
        }

        return current;
    }
};

template<typename T> struct unsorted_poly_node {

    T                              value;
    unsorted_poly_node<T>*         parent   = nullptr;
    vector<unsorted_poly_node<T>*> children = (0);

    unsorted_poly_node(T value)
        : value(value) {}
    unsorted_poly_node(T value, unsorted_poly_node<T>* initial_children[],
                       size_t initial_size)
        : value(value)
        , children(initial_children, initial_size, initial_size) {}

    operator T() { return value; }

    unsorted_poly_node<T>*& child(size_t index) { return children[index]; }

    unsorted_poly_node<T>*& operator[](size_t index) { return children[index]; }

    void add_child(unsorted_poly_node<T>* new_child) {
        children.push_back(new_child);
    }

    void remove_child(size_t index) { children.erase(index); }
    void remove_child(unsorted_poly_node<T>* target) {
        for (size_t i = 0; i < children.size(); i++) {
            if (children[i] == target) {
                children.erase(i);
                return;
            }
        }
    }
};

template<typename T> class unsorted_poly_tree {
  public:
    using node = unsorted_poly_node<T>;
    node* root;

    unsorted_poly_tree(node* root)
        : root(root) {}

    int* path_to(node* target, int* buffer) {
        int   depth   = 0;
        node* current = target;

        while (current != root) {
            // Move to parent
            node* last = current;
            current    = current->parent;

            // Find index
            int i = 0;
            for (; i < current->children.size(); i++) {
                if (current->child[i] == last) { break; }
            }

            buffer[depth] = i;
            depth++;
        }

        // Need to reverse current buffer to be path from root
        for (int i = 0; i < depth; i++) {
            int temp                = buffer[i];
            buffer[i]               = buffer[(depth - i - 1)];
            buffer[(depth - i - 1)] = temp;
        }

        // Path is terminated with -1
        buffer[depth] = -1;

        return buffer;
    }

    node* node_at(const int* path) {
        node* current = root;

        int i = 0;
        while (path[i] >= 0 && current != nullptr) {
            current = current->children[path[i]];
        }

        return current;
    }
};

} // namespace std_k

#endif // PINTOS_POLY_TREE_H

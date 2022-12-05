#ifndef PINTOS_DEVICE_TREE_H
#define PINTOS_DEVICE_TREE_H

#include "libk/cstring.h"
#include "libk/poly_tree.h"

struct device;
struct device_pointer_compare;

namespace devices {

struct device_tree_t {
    using node = std_k::poly_node<device*, device_pointer_compare>;
    static node& root;

    char* path_to(node* target, char* buffer);
    node* node_at(const char* path);
    node* node_at(node* directory, const char* path);

    node* add_node(node* target, const char* parent_path);
};
using device_tree_node = device_tree_t::node;

extern device_tree_t device_tree;

void register_device(device* new_device, const char* path,
                     device_tree_t* tree = &device_tree);

device* find_device(const char* target_path, unsigned int index,
                    device_tree_t* tree = &device_tree);

} // namespace devices

#endif // PINTOS_DEVICE_TREE_H

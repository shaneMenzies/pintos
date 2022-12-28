/**
 * @file device_tree.cpp
 * @author Shane Menzies
 * @brief
 * @date 10/23/22
 *
 *
 */

#include "device_tree.h"

#include "device.h"
#include "libk/cstring.h"
#include "libk/string.h"

namespace devices {

device               root_device = device("/", "", nullptr, 0);
device_tree_t::node& device_tree_t::root(root_device.tree_node);

device_tree_t device_tree;

char* device_tree_t::path_to(node* target, char* buffer) {
    size_t buffer_index = 0;
    node*  current      = target;

    // Place target name first
    size_t name_size = std_k::strlen(current->value->name) - 1;
    std_k::strnrev(buffer, current->value->name, name_size);
    buffer_index += name_size;
    buffer[buffer_index] = '/';
    buffer_index++;

    // Move up until root reached
    while (current != &root) {
        // Move to parent
        current = current->parent;

        name_size = std_k::strlen(current->value->name) - 1;
        std_k::strnrev(&buffer[buffer_index], current->value->name, name_size);
        buffer_index += name_size;
        buffer[buffer_index] = '/';
        buffer_index++;
    }
    buffer[buffer_index] = '\0';

    // Root reached, just need to reverse entire path
    std_k::strrev(buffer, buffer);
    return buffer;
}

device_tree_t::node* device_tree_t::node_at(const char* path) {

    // Path starts at root, with the first backslash
    node*  current = &root;
    size_t index   = 1;
    while (path[index] != '\0') {
        char buffer[256];

        // Find end of this name
        size_t length = 0;
        while (path[index + length] != '/' && path[index + length] != '\0') {
            length++;
        }

        // Make temporary copy
        std_k::strncpy(buffer, &path[index], length);

        // Find match in children
        node* match = nullptr;
        int   left  = 0;
        int   right = current->children.size() - 1;
        while (left <= right) {
            int middle = (left + right) / 2;
            int result
                = std_k::strcmp(current->children[middle]->value->name, buffer);

            if (result < 0) {
                left = middle + 1;
            } else if (result > 0) {
                right = middle - 1;
            } else {
                match = current->children[middle];
                break;
            }
        }

        if (match == nullptr) { return nullptr; }

        // Found match, move to it
        index += length;
        if (path[index] == '/') { index++; }
        current = match;
    }

    return current;
}

device_tree_t::node* device_tree_t::node_at(node* directory, const char* path) {

    // Path starts at given directory, path may start with backslash
    node*  current = directory;
    size_t index   = (path[0] == '/') ? 1 : 0;
    while (path[index] != '\0') {
        char buffer[256];

        // Find end of this name
        size_t length = 0;
        while (path[index + length] != '/' && path[index + length] != '\0') {
            length++;
        }

        // Make temporary copy
        std_k::strncpy(buffer, &path[index], length);

        // Find match in children
        node* match = nullptr;
        int   left  = 0;
        int   right = current->children.size() - 1;
        while (left <= right) {
            int middle = (left + right) / 2;
            int result
                = std_k::strcmp(current->children[middle]->value->name, buffer);

            if (result < 0) {
                left = middle + 1;
            } else if (result > 0) {
                right = middle - 1;
            } else {
                match = current->children[middle];
                break;
            }
        }

        if (match == nullptr) { return nullptr; }

        // Found match, move to it
        index += length;
        if (path[index] == '/') { index++; }
        current = match;
    }

    return current;
}

device_tree_t::node* device_tree_t::add_node(node*       target,
                                             const char* parent_path) {
    // Need to find parent first
    node* parent = node_at(parent_path);

    // Can add new child
    parent->add_child(target);
    return target;
}

void register_device(device* new_device, const char* path,
                     device_tree_t* tree) {
    device_tree_t::node* directory = tree->node_at(path);

    if (directory == nullptr) { return; }

    // Need to add correct index to end of device name
    unsigned int  duplicate_count = 0;
    std_k::string name_buffer     = new_device->name;
    size_t        name_end        = name_buffer.end();
    name_buffer.resize(name_buffer.size() + 16);
    while (true) {
        // Convert number to characters
        std_k::sprintf(&name_buffer[name_end], "%u", duplicate_count);

        // Check if this name is free
        if (tree->node_at(directory, name_buffer) == nullptr) { break; }

        duplicate_count++;
    }

    new_device->name.resize(strlen(name_buffer));
    strcpy(new_device->name, name_buffer);

    directory->add_child(&new_device->tree_node);
}

device* find_device(const char* target_path, unsigned int index,
                    device_tree_t* tree) {

    // Temporary buffer for real path
    std_k::string path_buffer = target_path;
    size_t        path_end    = path_buffer.end();
    path_buffer.resize(path_buffer.size() + 16);

    // Convert number to characters
    std_k::sprintf(&path_buffer[path_end], "%u", index);

    // Return value at this path
    device_tree_node* target_node = tree->node_at(path_buffer);
    if (target_node != nullptr) {
        return target_node->value;
    } else {
        return nullptr;
    }
}

} // namespace devices

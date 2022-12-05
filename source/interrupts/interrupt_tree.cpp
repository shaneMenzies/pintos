/**
 * @file interrupt_tree.cpp
 * @author Shane Menzies
 * @brief
 * @date 10/26/22
 *
 *
 */

#include "interrupt_tree.h"

#include "device/device_tree.h"

namespace interrupts {

interrupt_tree_t::node* root_interrupt_routes[256] = {nullptr};
interrupt_tree_t::node  root_interrupts(devices::device_tree.root.value,
                                        root_interrupt_routes, 256);

interrupt_tree_t interrupt_tree = (&root_interrupts);

void register_int_route(interrupt_tree_node* new_node, const int* path,
                        int index, interrupt_tree_t* tree) {
    interrupt_tree_t::node* directory = tree->node_at(path);

    if (directory == nullptr) { return; }

    directory->child(index) = new_node;
}

} // namespace interrupts

#ifndef PINTOS_INTERRUPT_TREE_H
#define PINTOS_INTERRUPT_TREE_H

#include "device/device.h"
#include "libk/poly_tree.h"

namespace interrupts {

using interrupt_tree_t    = std_k::unsorted_poly_tree<device*>;
using interrupt_tree_node = interrupt_tree_t::node;

extern interrupt_tree_t interrupt_tree;

/**
 * Add an interrupt route to an interrupt routing tree
 * @param new_node      Node to route this interrupt to
 * @param parent_path   Path to parent node
 * @param index         Index of parent to add this route
 * @param tree          Target tree
 */
void register_int_route(interrupt_tree_node* new_node, const int* parent_path,
                        int index, interrupt_tree_t* tree = &interrupt_tree);

} // namespace interrupts

#endif // PINTOS_INTERRUPT_TREE_H

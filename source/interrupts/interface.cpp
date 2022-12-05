/**
 * @file interface.cpp
 * @author Shane Menzies
 * @brief
 * @date 9/7/22
 *
 *
 */

#include "interface.h"

namespace interrupts {

uint8_t vector_alloc(interrupt_tree_node* owner) {
    for (uint8_t i = first_vector; i < 255; i++) {
        interrupt_tree_node* temp_null = nullptr;
        if (__atomic_compare_exchange_n(&interrupt_tree.root->children[i],
                                        &temp_null, owner, false,
                                        __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
            return i;
        };
    }
    return 0;
}

void vector_free(uint8_t vector) {
    interrupt_tree.root->children[vector] = nullptr;
}

uint8_t vector_override(uint8_t vector, interrupt_tree_node* owner) {
    interrupt_tree.root->children[vector] = owner;
    return vector;
}

} // namespace interrupts

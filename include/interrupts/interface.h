#ifndef PINTOS_INTERRUPTS_INTERFACE_H
#define PINTOS_INTERRUPTS_INTERFACE_H

#include "device/device.h"
#include "interrupt_tree.h"
#include "libk/common.h"

namespace interrupts {

const uint8_t first_vector = 0x20;

/**
 *  Requests ownership of an interrupt vector
 * @param owner     Device requesting ownership
 * @return  Interrupt vector allocated to this device
 */
uint8_t vector_alloc(interrupt_tree_node* owner);

/**
 *  Makes an interrupt vector available, removing current ownership
 * @param vector    Target interrupt vector
 */
void vector_free(uint8_t vector);

/**
 *  Sets ownership of a specific interrupt vector, regardless of current status
 * @param vector    Target interrupt vector
 * @param owner     New Owner
 * @return          Set interrupt vector (same as target)
 */
uint8_t vector_override(uint8_t vector, interrupt_tree_node* owner);

} // namespace interrupts

#endif // PINTOS_INTERRUPTS_INTERFACE_H

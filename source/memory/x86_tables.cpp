/**
 * @file x86_tables.cpp
 * @author Shane Menzies
 * @brief Various tables necessary for x86 processors
 * @date 03/11/21
 *
 *
 */

#include "x86_tables.h"

namespace x86_tables {

/**
 * @brief Get the offset of the target IDT Gate
 *
 * @param target    Target IDT Gate
 * @return void*    Offset of the target gate
 */
void* get_offset(const idt_gate target) {
    return (void*)(((uint64_t)target.offset_3 << 32)
                   | ((uint64_t)target.offset_2 << 16)
                   | (uint64_t)target.offset_1);
}

/**
 * @brief Set the offset of the target IDT Gate
 *
 * @param target        Target IDT Gate
 * @param new_offset    New offset for the target gate
 */
void set_offset(idt_gate& target, void* new_offset) {
    target.offset_1 = (uint16_t)((uintptr_t)new_offset & 0xffff);
    target.offset_2 = (uint16_t)(((uintptr_t)new_offset >> 16) & 0xffff);
}

/**
 * @brief Set all of the values of a certain IDT gate descriptor
 *
 * @param target        IDT Gate to be modified
 * @param offset        Offset of interrupt handler
 * @param gdt_selector  GDT Index of a segment selector
 * @param min_privilege Minimum privilege level for the handler
 * @param type          Type field
 * @param present       Whether this gate is valid or not
 * @param task_gate     Whether this is a task gate or not
 */
void set_gate(idt_gate& target, void* offset, gdt_selector selector,
              uint8_t min_privilege, uint8_t type, bool present) {

    uint8_t type_top = (((min_privilege << 1) & 0x6) | (present ? 8 : 0)) << 4;
    target.offset_1  = (uint16_t)((uintptr_t)offset & 0xffff);
    target.offset_2  = (uint16_t)(((uintptr_t)offset >> 16) & 0xffff);
    target.offset_3  = (uint32_t)(((uintptr_t)offset >> 32) & 0xffffffff);

    target.selector = selector;

    target.type_attr = ((type & 0xf) | (type_top & 0xf0));

    target.ist = 1;
}
} // namespace x86_tables

/**
 * @file io_apic.cpp
 * @author Shane Menzies
 * @brief
 * @date 8/15/22
 *
 *
 */

#include "io_apic.h"

#include "memory/addressing.h"
#include "threading/threading.h"

const uint64_t   num_io_apic_properties = 1;
device::property io_apic_properties[num_io_apic_properties]
    = {{device::property::predefined::serial_number, 0}};

io_apic::io_apic(acpi::madt_io_apic* source, acpi::madt_table* madt)
    : device(default_name, default_model, io_apic_properties,
             num_io_apic_properties)
    , target((volatile uint32_t*)(source->io_apic_addr | 0UL))
    , first_irq(source->global_int_base)
    , id(source->io_apic_id)
    // Children vector will be set to correct size later
    , int_tree_node(this, nullptr, 0) {

    // Identity map this device
    paging::kernel_address_space.identity_map_region((uintptr_t)target,
                                                     0x10 + sizeof(uint32_t));

    // Check how many irqs this IO_APIC can handle
    reg_select() = 0x1;
    last_irq     = (reg_data() >> 16) & 0xff;
    num_irqs     = last_irq - first_irq + 1;

    // Create children for interrupt tree
    int_tree_node.children
        = std_k::vector<interrupts::interrupt_tree_node*>(num_irqs);
    int_tree_node.children.resize(num_irqs);

    // Create mapping table
    current_mapping = new uint8_t[num_irqs];

    // Set defaults for all irqs
    for (uint8_t i = 0; i < num_irqs; i++) {
        set_irq(i, irq_entry(i + interrupts::IRQ_BASE));
    }

    // Process any source overrides
    acpi::madt_io_apic_src_override** overrides
        = new acpi::madt_io_apic_src_override*[num_irqs];
    int num_overrides
        = acpi::get_entries(madt, acpi::madt_entry_type::io_apic_src_override,
                            (acpi::entry_header**)(overrides), num_irqs);
    for (int i = 0; i < num_overrides; i++) {
        if ((overrides[i]->global_int > first_irq)
            && (overrides[i]->global_int < last_irq)) {
            irq_entry replacement
                = get_irq(overrides[i]->global_int - first_irq);
            replacement.int_vector
                = overrides[i]->irq_src + interrupts::IRQ_BASE;
            replacement.polarity     = (overrides[i]->flags & 2)
                                           ? irq_entry::active_low
                                           : irq_entry::active_high;
            replacement.trigger_mode = (overrides[i]->flags & 8)
                                           ? irq_entry::level
                                           : irq_entry::edge;
            replacement.owner
                = new interrupts::interrupt_tree_node(current_thread());
            replacement.destination = current_apic::get_id();

            if (overrides[i]->irq_src <= last_irq
                && overrides[i]->irq_src >= first_irq) {
                irq_entry alternate = get_irq(overrides[i]->irq_src);
                if (alternate.int_vector == overrides[i]->irq_src) {
                    alternate.int_vector
                        = ((overrides[i]->global_int - first_irq) & 0xff)
                          + interrupts::IRQ_BASE;
                    set_irq(overrides[i]->irq_src, alternate);
                }
            }

            set_irq((overrides[i]->global_int - first_irq) & 0xff, replacement);
        }
    }

    // Process any NMI entries
    acpi::madt_io_apic_nmi_src** nmi_entries
        = (acpi::madt_io_apic_nmi_src**)overrides;
    int num_entries
        = acpi::get_entries(madt, acpi::madt_entry_type::io_apic_nmi_src,
                            (acpi::entry_header**)(nmi_entries), num_irqs);
    for (int i = 0; i < num_entries; i++) {
        if ((nmi_entries[i]->global_int > first_irq)
            && (nmi_entries[i]->global_int < last_irq)) {
            irq_entry replacement
                = get_irq(nmi_entries[i]->global_int - first_irq);
            replacement.int_vector
                = nmi_entries[i]->nmi_src + interrupts::IRQ_BASE;
            replacement.delivery_mode = irq_entry::nmi;
            replacement.polarity      = (nmi_entries[i]->flags & 2)
                                            ? irq_entry::active_low
                                            : irq_entry::active_high;
            replacement.trigger_mode  = (nmi_entries[i]->flags & 8)
                                            ? irq_entry::level
                                            : irq_entry::edge;
            replacement.owner
                = new interrupts::interrupt_tree_node(current_thread());
            replacement.destination = current_apic::get_id();

            if (overrides[i]->irq_src <= last_irq
                && overrides[i]->irq_src >= first_irq) {
                irq_entry alternate = get_irq(overrides[i]->irq_src);
                if (alternate.int_vector == overrides[i]->irq_src) {
                    alternate.int_vector
                        = ((overrides[i]->global_int - first_irq) & 0xff)
                          + interrupts::IRQ_BASE;
                    set_irq(overrides[i]->irq_src, alternate);
                }
            }

            set_irq((nmi_entries[i]->global_int - first_irq) & 0xff,
                    replacement);
        }
    }

    delete[] overrides;

    // Register device in device tree
    devices::register_device(this, default_path, &devices::device_tree);
}

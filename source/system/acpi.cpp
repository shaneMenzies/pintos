/**
 * @file acpi.cpp
 * @author Shane Menzies
 * @brief ACPI table functions
 * @date 07/13/21
 *
 *
 */

#include "system/acpi.h"

#include "memory/addressing.h"
#include "system/pintos_std.h"
#include "terminal/terminal.h"

namespace acpi {

/**
 * @brief Finds the RSDP (Root System Descriptor Pointer)
 *
 * @param mb_info       Boot Info
 * @return old_rsdp*    Pointer to the RSDP
 */
old_rsdp* find_rsdp(multiboot_boot_info* mb_info) {

    old_rsdp* rsdp = 0;

    // Check to see if GRUB gaves us a copy of the rsdp
    if ((uintptr_t)mb_info->acpi_new_tag) {
        rsdp = (old_rsdp*)(&(mb_info->acpi_new_tag->rsdp));
        active_terminal->tprintf(
            "Found New ACPI RSDP Tag in Multiboot Info: 0x%p \n", (void*)rsdp);
        active_terminal->update();
    } else if ((uintptr_t)mb_info->acpi_old_tag) {
        rsdp = (old_rsdp*)(&(mb_info->acpi_old_tag->rsdp));
        active_terminal->tprintf(
            "Found Old ACPI RSDP Tag in Multiboot Info: 0x%p \n", (void*)rsdp);
        active_terminal->update();
    } else {

        // Need to manually search for the RSDP in memory
        active_terminal->write_s(
            "No ACPI RSDP Tag provided by Multiboot Info, starting Manual "
            "Search... \n");
        active_terminal->update();

        // Try to search for it in the EBDA
        uint64_t* current
            = (uint64_t*)((*((uint16_t* volatile)0x40e) | 0ULL) << 4);
        uintptr_t end = (uintptr_t)current + 0x400;
        if ((uintptr_t)current < 0x100000) {
            while ((uintptr_t)current < end) {
                if (*current == RSDP_SIGNATURE) {
                    rsdp = (old_rsdp*)current;
                    break;
                } else {
                    current = (uint64_t*)((uintptr_t)current + 1);
                }
            }
        }

        // Look near the end of lower memory
        current = (uint64_t*)0xe0000;
        end     = 0xfffff;
        while ((uintptr_t)current < end) {
            if (*current == RSDP_SIGNATURE) {
                rsdp = (old_rsdp*)current;
                break;
            } else {
                current = (uint64_t*)((uintptr_t)current + 1);
            }
        }

        active_terminal->tprintf("Manual Search for ACPI RSDP result: 0x%p \n",
                                 (void*)rsdp);
        active_terminal->update();
    }

    if ((uintptr_t)rsdp) {
        // Map the rsdp
        paging::kernel_address_space.identity_map_page((uintptr_t)rsdp);

        if (rsdp->is_modern()) {
            // XSDT
            xsdt* xsdt = ((modern_rsdp*)rsdp)->xsdt_addr;
            paging::kernel_address_space.identity_map_page((uintptr_t)xsdt);
            paging::kernel_address_space.identity_map_region((uintptr_t)xsdt,
                                                             xsdt->length);
            active_terminal->tprintf(
                "ACPI Master Table Found\nSignature: %c%c%c%c\nLength: "
                "%x\nACPI Revision: %u\nOEM ID: %c%c%c%c%c%c\n",
                xsdt->c_signature[0], xsdt->c_signature[1],
                xsdt->c_signature[2], xsdt->c_signature[3], xsdt->length,
                xsdt->revision, xsdt->oem_id[0], xsdt->oem_id[1],
                xsdt->oem_id[2], xsdt->oem_id[3], xsdt->oem_id[4],
                xsdt->oem_id[5]);
            if (!xsdt->valid_checksum()) { return 0; }
        } else {
            // RSDT
            rsdt* rsdt = rsdp->get_rsdt();
            paging::kernel_address_space.identity_map_page((uintptr_t)rsdt);
            paging::kernel_address_space.identity_map_region((uintptr_t)rsdt,
                                                             rsdt->length);
            active_terminal->tprintf(
                "ACPI Master Table Found\nSignature: %c%c%c%c\nLength: "
                "%x\nACPI Revision: %u\nOEM ID: %c%c%c%c%c%c\n",
                rsdt->c_signature[0], rsdt->c_signature[1],
                rsdt->c_signature[2], rsdt->c_signature[3], rsdt->length,
                rsdt->revision, rsdt->oem_id[0], rsdt->oem_id[1],
                rsdt->oem_id[2], rsdt->oem_id[3], rsdt->oem_id[4],
                rsdt->oem_id[5]);
            if (!rsdt->valid_checksum()) { return 0; }
        }

        // Before returning the rsdp, make sure all the tables are mapped
        map_tables(rsdp);
    }

    // Return the rsdp
    return rsdp;
}

/**
 * @brief Maps all of the ACPI tables into memory
 *
 * @param rsdp              System RSDP
 */
void map_tables(old_rsdp* rsdp) {

    // RSDT or XSDT?
    if (rsdp->is_modern()) {
        // XSDT
        xsdt* xsdt        = ((modern_rsdp*)rsdp)->xsdt_addr;
        int   num_entries = (xsdt->length - sizeof(table_header)) / 8;
        for (int i = 0; i < num_entries; i++) {
            // Map header
            paging::kernel_address_space.identity_map_page(
                (uintptr_t)xsdt->tables[i]);
            // Map entirety
            paging::kernel_address_space.identity_map_region(
                (uintptr_t)xsdt->tables[i], xsdt->tables[i]->length);
        }
    } else {
        // RSDT
        rsdt* rsdt        = rsdp->get_rsdt();
        int   num_entries = (rsdt->length - sizeof(table_header)) / 4;
        for (int i = 0; i < num_entries; i++) {
            // Map header
            paging::kernel_address_space.identity_map_page(
                (uintptr_t)rsdt->tables[i]);
            // Map entirety
            paging::kernel_address_space.identity_map_region(
                (uintptr_t)rsdt->tables[i],
                ((table_header*)(rsdt->tables[i] | 0ULL))->length);
        }
    }
}

/**
 * @brief Finds the target table
 *
 * @param rsdp              System RSDP
 * @param target            Signature of the target table
 * @return table_header*    Pointer to requested table
 */
table_header* get_table(old_rsdp* rsdp, table_signature target) {

    // RSDT or XSDT?
    if (rsdp->is_modern()) {
        // XSDT
        xsdt* xsdt        = ((modern_rsdp*)rsdp)->xsdt_addr;
        int   num_entries = (xsdt->length - sizeof(table_header)) / 8;
        for (int i = 0; i < num_entries; i++) {
            if (xsdt->tables[i]->u_signature == target) {

                return (xsdt->tables[i]);
            }
        }
    } else {
        // RSDT
        rsdt* rsdt        = rsdp->get_rsdt();
        int   num_entries = (rsdt->length - sizeof(table_header)) / 4;
        for (int i = 0; i < num_entries; i++) {
            if (((table_header*)(rsdt->tables[i] | 0ULL))->u_signature
                == target) {
                return (table_header*)(rsdt->tables[i] | 0ULL);
            }
        }
    }

    // Requested table not found
    return 0;
}

/**
 * @brief Get the entries of a certain type from a target table
 *
 * @param target_table  ACPI Table to search
 * @param target_type   Type of entry to return
 * @param entry_buffer  Buffer of entry pointers to be filled
 * @param buffer_max    Maximum entries for the buffer
 * @return int          Number of entries found
 */
int get_entries(table_header* target_table, uint8_t target_type,
                entry_header** entry_buffer, int buffer_max) {

    // Find where the table ends
    uintptr_t     end_point = (uintptr_t)target_table + target_table->length;
    entry_header* current_entry = target_table->get_entry_start();
    int           buffer_index  = 0;

    // Loop through all entries
    while ((uintptr_t)current_entry < end_point) {
        if (current_entry->type == target_type) {
            if (buffer_index < buffer_max) {
                entry_buffer[buffer_index] = current_entry;
            }
            buffer_index++;
        }

        current_entry = (current_entry->next_entry());
    }

    return buffer_index;
}

/**
 * @brief Counts the number of entries of a certain type that are present in a
 *        target table
 *
 * @param target_table  Table to search
 * @param target_type   Type of entry to count
 * @return int          Number of matching entries found
 */
int count_entries(table_header* target_table, uint8_t target_type) {

    // Find where the table ends
    uintptr_t     end_point = (uintptr_t)target_table + target_table->length;
    entry_header* current_entry = target_table->get_entry_start();
    int           buffer_index  = 0;

    // Loop through all entries
    while ((uintptr_t)current_entry < end_point) {
        if (current_entry->type == target_type) { buffer_index++; }
        current_entry = (current_entry->next_entry());
    }

    return buffer_index;
}

} // namespace acpi

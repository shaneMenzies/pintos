#ifndef P_SUB_MEMORY_H
#define P_SUB_MEMORY_H

#include "libk/misc.h"
#include "memory/p_memory.h"
#include "stddef.h"
#include "stdint.h"

#define SUB_PAGE_INFO_SIZE_PER_BYTE 16
#define SUB_PAGE_INFO_DATA_OFFSET   12
#define SUB_PAGE_INFO_SIZE                                 \
    (unsigned int)((PAGE_SIZE - SUB_PAGE_INFO_DATA_OFFSET) \
                   / (SUB_PAGE_INFO_SIZE_PER_BYTE + 1))
#define SUB_PAGE_DATA_PER_PAGE \
    ((PAGE_SIZE - SUB_PAGE_INFO_DATA_OFFSET) - SUB_PAGE_INFO_SIZE)

struct allocation_entry;

struct sub_mem_region_info {
    uint8_t entry[SUB_PAGE_INFO_SIZE];

    sub_mem_region_info() {
        for (unsigned int i = 0; i < SUB_PAGE_INFO_SIZE; i++) { entry[i] = 0; }
    }
    sub_mem_region_info(sub_mem_region_info& source) {
        for (unsigned int i = 0; i < SUB_PAGE_INFO_SIZE; i++) {
            entry[i] = source.entry[i];
        }
    }

    unsigned int find_free_space(size_t entries_needed) {

        uint32_t base_index    = 0;
        uint32_t current_index = base_index;
        uint32_t entries_found = 0;

        while (current_index < SUB_PAGE_INFO_SIZE) {
            if (entry[current_index] == 0) {
                entries_found++;

                if (entries_found == entries_needed) { return base_index; }

                current_index++;
            } else {
                entries_found = 0;

                // Move to next index
                base_index += entry[current_index];
                current_index = base_index;
            }
        }

        return ~(0);
    }

    unsigned int find_aligned_space(size_t   entries_needed,
                                    size_t   entry_alignment,
                                    uint32_t initial_offset) {

        uint32_t base_index    = initial_offset;
        uint32_t current_index = base_index;
        uint32_t entries_found = 0;

        while (current_index < SUB_PAGE_INFO_SIZE) {
            if (entry[current_index] == 0) {
                entries_found++;

                if (entries_found == entries_needed) { return base_index; }

                current_index++;
            } else {
                entries_found = 0;

                // Move to next aligned index
                base_index += entry[current_index];
                base_index
                    = (std_k::round_up_multiple((base_index - initial_offset),
                                                entry_alignment))
                      + initial_offset;
                current_index = base_index;
            }
        }

        return ~(0);
    }

    unsigned int allocate_space(size_t entries_needed) {
        unsigned int target_index = find_free_space(entries_needed);

        if (target_index == (unsigned int)~(0)) { return (unsigned int)~(0); }

        // Set this entry to allocated
        entry[target_index] = entries_needed & 0xff;

        return target_index;
    }

    unsigned int allocate_aligned_space(size_t entries_needed, size_t alignment,
                                        uint32_t initial_offset) {
        unsigned int target_index
            = find_aligned_space(entries_needed, alignment, initial_offset);

        if (target_index == (unsigned int)~(0)) { return (unsigned int)~(0); }

        // Set this entry to allocated
        entry[target_index] = entries_needed & 0xff;

        return target_index;
    }

    void return_space(unsigned int index) {
        // Unallocate this entry
        entry[index] = 0;
    };
} __attribute__((packed));

struct sub_mem_region {
    uint8_t             data[SUB_PAGE_DATA_PER_PAGE];
    sub_mem_region*     next_region      = 0;
    uint32_t            total_space_free = SUB_PAGE_DATA_PER_PAGE;
    sub_mem_region_info info;

    sub_mem_region() {}
    sub_mem_region(sub_mem_region& source)
        : next_region(source.next_region)
        , total_space_free(source.total_space_free)
        , info(source.info) {
        for (unsigned int i = 0; i < SUB_PAGE_DATA_PER_PAGE; i++) {
            data[i] = source.data[i];
        }
    }

    void* operator new(size_t size) {
        return aligned_alloc(size, SUB_PAGE_INFO_SIZE_PER_BYTE);
    }

    void* allocate_space(size_t size) {

        // Convert size to number of entries
        size_t entries = (size / SUB_PAGE_INFO_SIZE_PER_BYTE)
                         + ((size % SUB_PAGE_INFO_SIZE_PER_BYTE) ? 1 : 0);
        unsigned int target_index = info.allocate_space(entries);

        if (target_index == (unsigned int)~(0)) { return (void*)0; }

        total_space_free -= (size);
        return (void*)(&data[target_index * SUB_PAGE_INFO_SIZE_PER_BYTE]);
    }

    void* allocate_aligned_space(size_t size, size_t alignment) {

        // Convert size and alignment to number of entries
        size_t entries = (size / SUB_PAGE_INFO_SIZE_PER_BYTE)
                         + ((size % SUB_PAGE_INFO_SIZE_PER_BYTE) ? 1 : 0);
        size_t entry_alignment
            = (alignment / SUB_PAGE_INFO_SIZE_PER_BYTE)
              + ((alignment % SUB_PAGE_INFO_SIZE_PER_BYTE) ? 1 : 0);

        uint32_t offset
            = (std_k::round_up_multiple(((uintptr_t)&data[0]), alignment)
               - (uintptr_t)&data[0])
              / SUB_PAGE_INFO_SIZE_PER_BYTE;

        unsigned int target_index
            = info.allocate_aligned_space(entries, entry_alignment, offset);

        if (target_index == (unsigned int)~(0)) { return (void*)0; }

        total_space_free -= (size);
        return (void*)(&data[target_index * SUB_PAGE_INFO_SIZE_PER_BYTE]);
    }

    void free_space(void* target) {
        // Determine target's index
        unsigned int target_index
            = (unsigned int)((uintptr_t)target - (uintptr_t)&data[0])
              / SUB_PAGE_INFO_SIZE_PER_BYTE;
        info.return_space(target_index);
    }

} __attribute__((packed));

class sub_mem_manager {
    sub_mem_region first_region;
    uintptr_t      earliest_address;
    uintptr_t      latest_address;

  public:
    sub_mem_manager() {
        earliest_address = (uintptr_t)&first_region;
        latest_address   = earliest_address + PAGE_SIZE;
    }
    sub_mem_manager(sub_mem_manager& source)
        : first_region(source.first_region)
        , earliest_address(source.earliest_address)
        , latest_address(source.latest_address) {}

    inline bool owns_allocation(uintptr_t address) {
        // Check rough bounds
        if (address >= earliest_address && address <= latest_address) {
            sub_mem_region* current_region = &first_region;

            while (1) {
                if (address >= (uintptr_t)current_region
                    && address <= ((uintptr_t)current_region + PAGE_SIZE))
                    return true;

                if ((uintptr_t)current_region->next_region)
                    current_region = current_region->next_region;
                else
                    return false;
            }
        } else {
            return false;
        }
    }

    void* sub_alloc(size_t size);
    void* sub_aligned_alloc(size_t size, size_t alignment);

    bool try_sub_free(uintptr_t address);
};

#endif

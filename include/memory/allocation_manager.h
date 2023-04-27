#ifndef ALLOCATION_MANAGER_H
#define ALLOCATION_MANAGER_H

#include "libk/asm.h"
#include "libk/avl_tree.h"
#include "memory/chunking.h"

#include <stddef.h>
#include <stdint.h>

class allocation_manager;

extern allocation_manager kernel_allocation_manager;

struct allocation_info {
    uintptr_t        address;
    chunking::chunk* chunks;
    unsigned int     num_chunks;

    allocation_info(uintptr_t address = 0, chunking::chunk* chunks = 0,
                    unsigned int num_chunks = 0)
        : address(address)
        , chunks(chunks)
        , num_chunks(num_chunks) {}

    friend bool operator==(const allocation_info& lhs,
                           const allocation_info& rhs) {
        return (lhs.address == rhs.address);
    }
    friend bool operator!=(const allocation_info& lhs,
                           const allocation_info& rhs) {
        return !(lhs == rhs);
    }
    friend bool operator<(const allocation_info& lhs,
                          const allocation_info& rhs) {
        return (lhs.address < rhs.address);
    }
    friend bool operator>(const allocation_info& lhs,
                          const allocation_info& rhs) {
        return rhs < lhs;
    }
    friend bool operator<=(const allocation_info& lhs,
                           const allocation_info& rhs) {
        return !(lhs > rhs);
    }
    friend bool operator>=(const allocation_info& lhs,
                           const allocation_info& rhs) {
        return !(lhs < rhs);
    }
};

struct allocation_entry : public std_k::avl_node<allocation_info> {

    allocation_entry(uintptr_t address = 0, chunking::chunk* chunks = 0,
                     unsigned int num_chunks = 0) {
        value.address    = address;
        value.chunks     = chunks;
        value.num_chunks = num_chunks;
    }

    allocation_entry(allocation_info source) { value = source; }
};

class allocation_manager {
  public:
    std_k::avl_tree<allocation_info> info_map;

    std_k::mutex entry_lock;

    allocation_manager() {};

    void add_entry(allocation_entry* entry) {

        // Get modification lock
        entry_lock.lock();

        // Add into the map
        info_map.insert(entry);

        // Release lock
        entry_lock.unlock();
    }

    allocation_info take_entry(uintptr_t address) {

        // Get lock
        entry_lock.lock();

        // Find the entry
        allocation_entry* target
            = (allocation_entry*)info_map.find(allocation_info(address, 0, 0));

        if (target == nullptr) {
            entry_lock.unlock();
            return allocation_info();
        } else {
            allocation_info value = target->value;

            // Remove it from the map
            info_map.seperate(target);

            entry_lock.unlock();
            return value;
        }
    }
};

#endif

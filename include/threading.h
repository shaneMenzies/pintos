#ifndef THREADING_H
#define THREADING_H

#include <cpuid.h>

#include "pintos_std.h"
#include "acpi.h"
#include "p_memory.h"
#include "libk.h"

namespace threading {

    struct logical_core {
        uint32_t apic_id;
        bool functional;

        uint32_t logical_index;
        uint32_t physical_index;
        uint32_t socket_index;
        uint32_t domain_index;

        friend bool operator==(logical_core& lhs, logical_core& rhs) {
            if (lhs.apic_id != rhs.apic_id 
                || lhs.domain_index != rhs.domain_index
                || lhs.socket_index != rhs.socket_index 
                || lhs.physical_index != rhs.physical_index
                || lhs.logical_index != rhs.logical_index) {
                return false;
            } else {
                return true;
            }
        }
        friend bool operator!=(logical_core& lhs, logical_core& rhs) {return !(lhs == rhs);}

        friend bool operator<(logical_core& lhs, logical_core& rhs) {
            /*
             *  Recursively test each index along the heirarchy:
             *      1. NUMA Domain
             *      2. Socket/Chip Index
             *      3. Physical Core Index
             *      4. Logical Core Index
             *      5. APIC ID
             *
             */
            if (lhs.domain_index < rhs.domain_index 
                || (lhs.domain_index == rhs.domain_index && (lhs.socket_index < rhs.socket_index
                || (lhs.socket_index == rhs.socket_index && (lhs.physical_index < rhs.physical_index 
                || (lhs.physical_index == rhs.physical_index && (lhs.logical_index < rhs.logical_index 
                || (lhs.logical_index == rhs.logical_index && lhs.apic_id < rhs.apic_id)))))))) {
                return true;
            } else {
                return false;
            }
        }
        friend bool operator> (logical_core& lhs, logical_core& rhs){ return rhs < lhs; }
        friend bool operator<=(logical_core& lhs, logical_core& rhs){ return !(lhs > rhs); }
        friend bool operator>=(logical_core& lhs, logical_core& rhs){ return !(lhs < rhs); }
    };

    struct physical_core {
        unsigned int num_logical;
        logical_core* logical;
    };

    struct socket {
        unsigned int num_physical;
        physical_core* physical;

        uint32_t id;
    };

    struct numa_domain {
        unsigned int num_socket;
        socket* sockets;

        uintptr_t start;
        uintptr_t end;
    };

    struct system {
        unsigned int num_domains;
        numa_domain* domain;

        unsigned int total_threads;
        unsigned int total_cores;
        unsigned int total_sockets;
    };

    extern system topology;

    void detect_topology(acpi::madt_table* madt, acpi::srat_table* srat);

    void new_thread_startup();
}

#endif
#ifndef PINTOS_TOPOLOGY_H
#define PINTOS_TOPOLOGY_H

#include "apic.h"
#include "device/device.h"
#include "libk/asm.h"
#include "system/acpi.h"

namespace threading {
struct thread_scheduler;
}

namespace chunking {
struct chunk_pile;
}

struct logical_core : public device {
    bool functional;
    bool x2apic_thread;
    bool boot_thread;

    apic<true, false>            local_apic;
    chunking::chunk_pile*        memory_piles;
    threading::thread_scheduler* scheduler;
    void*                        sys_stack;

    void start_thread(void (*target_code)());

    friend bool operator==(logical_core& lhs, logical_core& rhs) {
        return (lhs.local_apic.id == rhs.local_apic.id);
    }
    friend bool operator!=(logical_core& lhs, logical_core& rhs) {
        return !(lhs == rhs);
    }

    friend bool operator<(logical_core& lhs, logical_core& rhs) {
        return (lhs.local_apic.id < rhs.local_apic.id);
    }
    friend bool operator>(logical_core& lhs, logical_core& rhs) {
        return rhs < lhs;
    }
    friend bool operator<=(logical_core& lhs, logical_core& rhs) {
        return !(lhs > rhs);
    }
    friend bool operator>=(logical_core& lhs, logical_core& rhs) {
        return !(lhs < rhs);
    }
};

struct physical_core {
    unsigned int num_logical;
};

struct socket {
    unsigned int num_physical;
    uint32_t     id;
};

struct numa_domain {
    unsigned int num_socket;
    uintptr_t    start;
    uintptr_t    end;
};

struct system {
    unsigned int num_domains;
    unsigned int num_sockets;
    unsigned int num_physical;
    unsigned int num_logical;

    numa_domain*   domains;
    socket*        sockets;
    physical_core* cores;

    logical_core* threads;

    inline unsigned int get_total_index(apic_id id) {
        return (id.core_index() * num_logical) + id.thread_index();
    }
    inline logical_core* get_thread(apic_id id) {
        return &(threads[get_total_index((id))]);
    }
};

extern struct system topology;

inline logical_core* current_thread() {
    return topology.get_thread(current_apic::get_id());
}

void detect_topology(acpi::madt_table* madt, acpi::srat_table* srat);

#endif // PINTOS_TOPOLOGY_H

/**
 * @file threading.cpp
 * @author Shane Menzies
 * @brief Multithreading support
 * @date 07/27/21
 * 
 * 
 */

#include "threading.h"

namespace threading {

    struct new_thread_startup_info thread_startup_info;

    system topology;

    void halt() {
        asm volatile ("hlt");
    }

    void detect_topology(acpi::madt_table* madt, acpi::srat_table* srat) {

        // Determine number of logical cores from MADT entries
        uint32_t num_logical = 0;
        uint32_t num_physical = 0;
        uint32_t num_sockets = 0;
        uint32_t num_domains = 0;

        // Get and fill out an array of logical cores
        int total_entries = acpi::count_entries(madt, acpi::madt_entry_type::processor_apic)
                          + acpi::count_entries(madt, acpi::madt_entry_type::processor_x2apic);
        logical_core* logical_cores = (logical_core*)malloc(sizeof(logical_core) * total_entries);
        acpi::entry_header** acpi_entries = (acpi::entry_header**)malloc(sizeof(acpi::entry_header*) * total_entries);

        // Get the boot thread's apic id
        uint32_t boot_apic_id = *get_apic_register(0x20);

        // Check standard apic entries
        int num_entries = acpi::get_entries(madt, acpi::madt_entry_type::processor_apic, acpi_entries, total_entries);
        int i = 0;
        while (i < num_entries) {
            if ((((acpi::madt_processor_apic**)acpi_entries)[i]->flags & 1) || (((acpi::madt_processor_apic**)acpi_entries)[i]->flags & 2)) {
                logical_cores[num_logical].functional = true;
                logical_cores[num_logical].apic_id = ((acpi::madt_processor_apic**)acpi_entries)[i]->apic_id;
                logical_cores[num_logical].logical_index = 0;
                logical_cores[num_logical].physical_index = 0;
                logical_cores[num_logical].socket_index = 0;
                logical_cores[num_logical].domain_index = 0;
                logical_cores[num_logical].x2apic_thread = false;
                logical_cores[num_logical].boot_thread = (logical_cores[num_logical].apic_id == boot_apic_id);
                num_logical++;
            }
            i++;
        }

        // Check x2apic entries
        num_entries += acpi::get_entries(madt, acpi::madt_entry_type::processor_x2apic, &(acpi_entries[num_entries]), (total_entries - num_entries));
        while (i < num_entries) {
            if ((((acpi::madt_processor_x2apic**)acpi_entries)[i]->flags & 1) || (((acpi::madt_processor_x2apic**)acpi_entries)[i]->flags & 2)) {
                logical_cores[num_logical].functional = true;
                logical_cores[num_logical].apic_id = ((acpi::madt_processor_x2apic**)acpi_entries)[i]->processor_id;
                logical_cores[num_logical].logical_index = 0;
                logical_cores[num_logical].physical_index = 0;
                logical_cores[num_logical].socket_index = 0;
                logical_cores[num_logical].domain_index = 0;
                logical_cores[num_logical].x2apic_thread = true;
                logical_cores[num_logical].boot_thread = (logical_cores[num_logical].apic_id == boot_apic_id);
                num_logical++;
            }
            i++;
        }

        // Sort array of logical cores for easier matching by id
        sorts::insertion_sort<logical_core>(logical_cores, num_logical);

        // Check SRAT entries to determine numa domains
        if ((acpi::count_entries(srat, acpi::srat_entry_type::apic_affinity) > total_entries) || (acpi::count_entries(srat, acpi::srat_entry_type::x2apic_affinity) > total_entries)) {
            // Need to get a new entry buffer for the SRAT entries
            if (acpi::count_entries(srat, acpi::srat_entry_type::apic_affinity) > acpi::count_entries(srat, acpi::srat_entry_type::x2apic_affinity)) {
                total_entries = acpi::count_entries(srat, acpi::srat_entry_type::apic_affinity);
            } else {
                total_entries = acpi::count_entries(srat, acpi::srat_entry_type::x2apic_affinity);
            }
            free(acpi_entries);
            acpi_entries = (acpi::entry_header**)malloc(sizeof(acpi::entry_header*) * total_entries);
        } else {
            if (acpi::count_entries(srat, acpi::srat_entry_type::apic_affinity) > acpi::count_entries(srat, acpi::srat_entry_type::x2apic_affinity)) {
                total_entries = acpi::count_entries(srat, acpi::srat_entry_type::apic_affinity);
            } else {
                total_entries = acpi::count_entries(srat, acpi::srat_entry_type::x2apic_affinity);
            }
        }

        num_entries = acpi::get_entries(srat, acpi::srat_entry_type::apic_affinity, acpi_entries, num_entries);
        i = 0;
        while (i < num_entries) {
            if (((acpi::srat_apic_affinity**)acpi_entries)[i]->flags & 1) {
                logical_core target_core;
                target_core.apic_id = ((acpi::srat_apic_affinity**)acpi_entries)[i]->APIC_ID;
                int target_index = sorts::binary_match<logical_core>(logical_cores, num_logical, target_core);

                uint32_t domain = ((acpi::srat_apic_affinity**)acpi_entries)[i]->lo_DM 
                                  | (((acpi::srat_apic_affinity**)acpi_entries)[i]->hi_DM[0] << 8) 
                                  | (((acpi::srat_apic_affinity**)acpi_entries)[i]->hi_DM[1] << 16)
                                  | (((acpi::srat_apic_affinity**)acpi_entries)[i]->hi_DM[2] << 24);
                logical_cores[target_index].domain_index = domain;

                if ((domain + 1) > num_domains) {
                    num_domains = (domain + 1);
                }
            }
            i++;
        }

        // Check x2apic entries
        num_entries = acpi::get_entries(srat, acpi::srat_entry_type::x2apic_affinity, acpi_entries, num_entries);
        i = 0;
        while (i < num_entries) {
            if (((acpi::srat_x2apic_affinity**)acpi_entries)[i]->flags & 1) {
                logical_core target_core;
                target_core.apic_id = ((acpi::srat_x2apic_affinity**)acpi_entries)[i]->x2APIC_ID;
                int target_index = sorts::binary_match<logical_core>(logical_cores, num_logical, target_core);

                uint32_t domain = ((acpi::srat_x2apic_affinity**)acpi_entries)[i]->domain;
                logical_cores[target_index].domain_index = domain;

                if ((domain + 1) > num_domains) {
                    num_domains = (domain + 1);
                }
            }
            i++;
        }

        // Systems without NUMA domains have only 1 domain
        if (num_domains == 0) {
            num_domains = 1;
        }

        // Sort the entries again, now with info on numa domains
        sorts::insertion_sort<logical_core>(logical_cores, num_logical);

        // Can create topology info on numa domains
        topology.num_domains = num_domains;
        topology.domain = (numa_domain*)malloc(sizeof(numa_domain) * num_domains);
        for (unsigned int i = 0; i < num_domains; i++) {
            topology.domain[i].num_socket = 0;
            topology.domain[i].start = 0;
            topology.domain[i].end = ~(0);
        }

        // Process memory entries and apply those to the numa domains
        num_entries = acpi::get_entries(srat, acpi::srat_entry_type::mem_affinity, acpi_entries, num_entries);
        i = 0;
        while (i < num_entries) {
            if (((acpi::srat_mem_affinity**)acpi_entries)[i]->flags & 1) {
                topology.domain[((acpi::srat_mem_affinity**)acpi_entries)[i]->domain].start = (
                    ((acpi::srat_mem_affinity**)acpi_entries)[i]->lo_base 
                    | 0ULL 
                    | ((((acpi::srat_mem_affinity**)acpi_entries)[i]->hi_base | 0ULL) << 32)
                ); 

                topology.domain[((acpi::srat_mem_affinity**)acpi_entries)[i]->domain].end = (
                    topology.domain[((acpi::srat_mem_affinity**)acpi_entries)[i]->domain].start
                    + (((acpi::srat_mem_affinity**)acpi_entries)[i]->lo_length 
                    | 0ULL 
                    | ((((acpi::srat_mem_affinity**)acpi_entries)[i]->hi_length | 0ULL) << 32))
                );
            }

            i++;
        }

        // Detection of layout of sockets and physical/logical cores 
        // is different between AMD and Intel CPUs (other vendors are
        // assumed to follow Intel for simplicity's sake)
        uint32_t id_logical_bits = 0;
        uint32_t id_physical_bits = 0;
        uint32_t max_supported_cpuid = __get_cpuid_max(0U, (unsigned int*)0);
        uint32_t max_supported_ext = __get_cpuid_max(0x80000000, 0);

        // Check the HTT flag (if false, then its one core, one thread)
        uint32_t eax, ebx = 0, ecx = 0, edx = 0;
        eax = 1;
        __get_cpuid(1, &eax, &ebx, &ecx, &edx);
        if ((edx & (1<<28)) || num_logical > 1) {
            uint32_t thread_count = ((ebx & (0x00ff0000)) >> 16);

            if (get_vendor() == amd) {
                // AMD Procedure

                // If supported, use cpuid 0x80000008
                if (max_supported_ext >= 0x80000008) {
                    eax = 0x80000008;
                    __get_cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
                    if (ecx & (0xf000)) {
                        id_physical_bits = (ecx & 0xf000) >> 12;
                    } else {
                        id_physical_bits = round_next_binary_power((uint32_t)(ecx & 0xff));
                    }

                    id_logical_bits = round_next_binary_power(thread_count >> id_physical_bits);
                } else {
                // Otherwise revert to cpuid 0x01 results
                    id_logical_bits = 0;
                    id_physical_bits = round_next_binary_power(thread_count);
                }

            } else {
                // Intel Procedure

                // If supported, use cpuid 0x0b
                if (max_supported_cpuid >= 0x0b) {
                    ecx = 0;
                    eax = 0x0b;
                    __get_cpuid_count(0x0b, 0, &eax, &ebx, &ecx, &edx);
                    id_logical_bits = eax & 0x1f;
                    ecx = 1;
                    eax = 0x0b;
                    __get_cpuid_count(0x0b, 1, &eax, &ebx, &ecx, &edx);
                    id_physical_bits = (eax & 0x1f) - id_logical_bits;
                } else if (max_supported_cpuid >= 0x04) {
                    eax = 0x04;
                    __get_cpuid(0x04, &eax, &ebx, &ecx, &edx);
                    uint32_t core_count = (eax >> 26) & 0x3f;
                    uint32_t count = 0;
                    while (core_count) {
                        core_count >>= 1;
                        count++;
                    }
                    id_physical_bits = count;

                    thread_count--;
                    count = 0;
                    while (thread_count) {
                        thread_count >>= 1;
                        count++;
                    }
                    id_logical_bits = count - id_physical_bits;
                } else {
                    id_logical_bits = round_next_binary_power(thread_count);
                }
            } 
        }
        
        // Can now tell a thread's position from it's APIC ID
        uint32_t* socket_id = (uint32_t*)malloc(sizeof(uint32_t) * num_logical * 3);
        uint32_t* socket_index = (uint32_t*)(&(socket_id[num_logical]));
        uint32_t* domain_index = (uint32_t*)(&(socket_id[(num_logical * 2)]));
        for (unsigned int i = 0; i < num_logical; i++) {
            socket_id[i] = ~(0);
            socket_index[i] = ~(0);
            domain_index[i] = ~(0);
        }
        for (unsigned int i = 0; i < num_logical; i++) {
            logical_cores[i].logical_index = logical_cores[i].apic_id &  ((1 << id_logical_bits) - 1);
            logical_cores[i].physical_index = (logical_cores[i].apic_id >> id_logical_bits) & ((1 << id_physical_bits) - 1);
            uint32_t current_socket = logical_cores[i].apic_id & ~((1 << (id_logical_bits + id_physical_bits)) - 1);
            int match_result = sorts::binary_match<uint32_t>(socket_id, num_logical, current_socket);
            if (match_result < 0) {
                logical_cores[i].socket_index= topology.domain[logical_cores[i].domain_index].num_socket;
                topology.domain[logical_cores[i].domain_index].num_socket++;
                socket_id[num_sockets] = current_socket;
                socket_index[num_sockets] = logical_cores[i].socket_index;
                domain_index[num_sockets] = logical_cores[i].domain_index;
                num_sockets++;
            } else {
                logical_cores[i].socket_index = socket_index[match_result];
            }
        }

        // Can create socket info 
        for (unsigned int i = 0; i < num_domains; i++) {
            topology.domain[i].sockets = (socket*)malloc(sizeof(socket) * topology.domain[i].num_socket);
            for (unsigned int j = 0; j < topology.domain[i].num_socket; j++) {

                // Find the correct socket id for this socket
                for (unsigned int index = 0; index < num_sockets; index++) {
                    if (domain_index[index] == i && socket_index[index] == j) {
                        topology.domain[i].sockets[j].id = socket_id[index];
                        break;
                    }
                }
            }
        }
        free(socket_id);
        
        // Final sorting of logical cores before connecting everything together 
        // into the topology structure
        sorts::insertion_sort<logical_core>(logical_cores, num_logical);

        // Connect all the sockets with their physical cores
        unsigned int logical_index = 1;
        while (logical_index <= num_logical) {
            if (logical_cores[logical_index].physical_index < logical_cores[logical_index - 1].physical_index || logical_index == num_logical) {
                // Socket just ended
                logical_index--;
                unsigned int next_num_physical = logical_cores[logical_index].physical_index + 1;
                topology.domain[logical_cores[logical_index].domain_index].sockets[logical_cores[logical_index].socket_index].physical
                    = new physical_core[next_num_physical];
                topology.domain[logical_cores[logical_index].domain_index].sockets[logical_cores[logical_index].socket_index].num_physical
                    = next_num_physical;
                logical_index++;

                num_physical += next_num_physical;
            }
            logical_index++;
        }

        // Connect all the physical cores with their logical threads
        logical_index = 1;
        while (logical_index <= num_logical) {
            if (logical_cores[logical_index].logical_index < logical_cores[logical_index - 1].logical_index || logical_index == num_logical) {
                // Physical core just ended
                logical_index--;
                unsigned int next_num_logical = logical_cores[logical_index].logical_index + 1;
                topology.domain[logical_cores[logical_index].domain_index].sockets[logical_cores[logical_index].socket_index].physical[logical_cores[logical_index].physical_index].logical 
                    =  &(logical_cores[logical_index - logical_cores[logical_index].logical_index]);
                topology.domain[logical_cores[logical_index].domain_index].sockets[logical_cores[logical_index].socket_index].physical[logical_cores[logical_index].physical_index].num_logical
                    = next_num_logical;
                logical_index++;
            }
            logical_index++;
        }

        topology.total_threads = num_logical;
        topology.total_cores = num_physical;
        topology.total_sockets = num_sockets;
    }

    void logical_core::start_thread(void (*target_code)()) {

        // Make sure we aren't targeting the current thread
        if (*(get_apic_register(0x20)) == apic_id) {
            boot_thread = true;
            return;
        }

        // Prepare startup environment for the new core
        *thread_startup_info.thread_target = (void*)(thread_spinlock);
        *thread_startup_info.thread_stack_top = (void*)((uintptr_t)malloc(32768) + 32768);

        // Send initialize assertion command
        send_apic_command(apic_id, 0, 5, false, true, true, 0);
        timer::sys_int_timer->sleep(0.001);

        // Send initialization de-assert command
        send_apic_command(apic_id, 0, 5, false, false, true, 0);
        timer::sys_int_timer->sleep(0.001);

        // Send interrupt to bootstrap code, and wait 1ms (0.0001 seconds)
        send_apic_command(apic_id, ((uintptr_t)thread_startup_info.thread_start / paging::page_size), 6, false, false, false, 0);
        timer::sys_int_timer->sleep(0.0001);

        // Send it again if the new thread hasn't reached the spinlock yet
        if (!spinlock_reached) {
            // Repeat the interrupt, waiting up to 1s this time
            send_apic_command(apic_id, ((uintptr_t)thread_startup_info.thread_start / paging::page_size), 6, false, false, false, 0);
            for (int i = 0; i < 100; i++) {
                if (spinlock_reached) {
                    break;
                } else {
                    timer::sys_int_timer->sleep(0.01);
                }
            }
        }

        if (spinlock_reached) {
            // Can set the new target and release the thread from it's spinlock
            after_spinlock_target = target_code;
            release_spinlock = true;
        }

    }

    void start_threads(void (*target)()) {
        for (unsigned int d = 0; d < topology.num_domains; d++) {
            // Loop through domains
            numa_domain current_domain = topology.domain[d];
            for (unsigned int s = 0; s < current_domain.num_socket; s++) {
                socket current_socket = current_domain.sockets[s];
                for (unsigned int p = 0; p < current_socket.num_physical; p++) {
                    physical_core core = current_socket.physical[p];
                    for (unsigned int l = 0; l < core.num_logical; l++) {
                        if (!core.logical[l].boot_thread) {
                            core.logical[l].start_thread(target);
                        }
                    }
                }
            }
        }
    }
}
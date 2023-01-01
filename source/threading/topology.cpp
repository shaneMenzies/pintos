/**
 * @file topology.cpp
 * @author Shane Menzies
 * @brief
 * @date 6/16/22
 *
 *
 */

#include "threading/topology.h"

#include "libk/misc.h"
#include "libk/sorting.h"
#include "memory/allocation_manager.h"
#include "memory/p_memory.h"
#include "system/acpi.h"
#include "system/kernel.h"
#include "threading.h"
#include "time/timer.h"

struct system topology;

void logical_core::start_thread(void (*target_code)()) {

    // Make sure we aren't targeting the current thread
    if (current_apic::get_id() == local_apic.id) {
        boot_thread = true;
        return;
    }

    // Prepare startup environment for the new core
    *threading::thread_startup_info.thread_target = (void*)(thread_spinlock);
    *threading::thread_startup_info.thread_stack_top
        = (void*)((uintptr_t)malloc(32768) + 32768);
    sys_stack = threading::thread_startup_info.thread_stack_top;

    // Send initialize assertion command
    current_apic::send_apic_command(local_apic.id, 0, 5, false, false, 0);
    sys_int_timer->sleep(0.001);

    // Send initialization de-assert command
    current_apic::send_apic_command(local_apic.id, 0, 5, false, true, 0);
    sys_int_timer->sleep(0.001);

    // Send interrupt to bootstrap code, and wait 1ms (0.0001 seconds)
    current_apic::send_apic_command(
        local_apic.id,
        ((uintptr_t)threading::thread_startup_info.thread_start / PAGE_SIZE), 6,
        false, false, 0);
    sys_int_timer->sleep(0.0001);

    // Send it again if the new thread hasn't reached the spinlock yet
    if (!spinlock_reached) {
        // Repeat the interrupt, waiting up to 1s this time
        current_apic::send_apic_command(
            local_apic.id,
            ((uintptr_t)threading::thread_startup_info.thread_start
             / PAGE_SIZE),
            6, false, false, 0);
        for (int i = 0; i < 100; i++) {
            if (spinlock_reached) {
                break;
            } else {
                sys_int_timer->sleep(0.01);
            }
        }
    }

    if (spinlock_reached) {
        // Can set the new target and release the thread from it's spinlock
        after_spinlock_target = target_code;
        release_spinlock      = true;
    }
}

void detect_topology(acpi::madt_table* madt, acpi::srat_table* srat) {

    // Determine number of logical cores from MADT entries
    unsigned int num_logical  = 0;
    unsigned int num_physical = 0;
    unsigned int num_sockets  = 0;
    unsigned int num_domains  = 0;

    // Get and fill out an array of logical cores
    int total_entries
        = acpi::count_entries(madt, acpi::madt_entry_type::processor_apic)
          + acpi::count_entries(madt, acpi::madt_entry_type::processor_x2apic);
    logical_core* logical_cores
        = (logical_core*)malloc(sizeof(logical_core) * total_entries);
    acpi::entry_header** acpi_entries = (acpi::entry_header**)malloc(
        sizeof(acpi::entry_header*) * total_entries);

    // Get the boot thread's apic id
    apic_id boot_apic_id = current_apic::get_id();

    // Check standard apic entries
    int num_entries
        = acpi::get_entries(madt, acpi::madt_entry_type::processor_apic,
                            acpi_entries, total_entries);
    int i = 0;
    while (i < num_entries) {
        if ((((acpi::madt_processor_apic**)acpi_entries)[i]->flags & 1)
            || (((acpi::madt_processor_apic**)acpi_entries)[i]->flags & 2)) {
            logical_cores[num_logical].functional = true;
            logical_cores[num_logical].local_apic.id
                = ((acpi::madt_processor_apic**)acpi_entries)[i]->apic_id;
            logical_cores[num_logical].x2apic_thread = false;
            logical_cores[num_logical].boot_thread
                = (logical_cores[num_logical].local_apic.id == boot_apic_id);
            num_logical++;
        }
        i++;
    }

    // Check x2apic entries
    num_entries += acpi::get_entries(
        madt, acpi::madt_entry_type::processor_x2apic,
        &(acpi_entries[num_entries]), (total_entries - num_entries));
    while (i < num_entries) {
        if ((((acpi::madt_processor_x2apic**)acpi_entries)[i]->flags & 1)
            || (((acpi::madt_processor_x2apic**)acpi_entries)[i]->flags & 2)) {
            logical_cores[num_logical].functional = true;
            logical_cores[num_logical].local_apic.id
                = ((acpi::madt_processor_x2apic**)acpi_entries)[i]
                      ->processor_id;
            logical_cores[num_logical].x2apic_thread = true;
            logical_cores[num_logical].boot_thread
                = (logical_cores[num_logical].local_apic.id == boot_apic_id);
            num_logical++;
        }
        i++;
    }

    // Sort array of logical cores for easier matching by id
    std_k::insertion_sort<logical_core>(logical_cores, num_logical);

    // Check SRAT entries to determine numa domains
    if ((acpi::count_entries(srat, acpi::srat_entry_type::apic_affinity)
         > total_entries)
        || (acpi::count_entries(srat, acpi::srat_entry_type::x2apic_affinity)
            > total_entries)) {
        // Need to get a new entry buffer for the SRAT entries
        if (acpi::count_entries(srat, acpi::srat_entry_type::apic_affinity)
            > acpi::count_entries(srat,
                                  acpi::srat_entry_type::x2apic_affinity)) {
            total_entries = acpi::count_entries(
                srat, acpi::srat_entry_type::apic_affinity);
        } else {
            total_entries = acpi::count_entries(
                srat, acpi::srat_entry_type::x2apic_affinity);
        }
        free(acpi_entries);
        acpi_entries = (acpi::entry_header**)malloc(sizeof(acpi::entry_header*)
                                                    * total_entries);
    } else {
        if (acpi::count_entries(srat, acpi::srat_entry_type::apic_affinity)
            > acpi::count_entries(srat,
                                  acpi::srat_entry_type::x2apic_affinity)) {
            total_entries = acpi::count_entries(
                srat, acpi::srat_entry_type::apic_affinity);
        } else {
            total_entries = acpi::count_entries(
                srat, acpi::srat_entry_type::x2apic_affinity);
        }
    }

    num_entries = acpi::get_entries(srat, acpi::srat_entry_type::apic_affinity,
                                    acpi_entries, num_entries);
    i           = 0;
    while (i < num_entries) {
        if (((acpi::srat_apic_affinity**)acpi_entries)[i]->flags & 1) {

            unsigned int domain
                = ((acpi::srat_apic_affinity**)acpi_entries)[i]->lo_DM
                  | (((acpi::srat_apic_affinity**)acpi_entries)[i]->hi_DM[0]
                     << 8)
                  | (((acpi::srat_apic_affinity**)acpi_entries)[i]->hi_DM[1]
                     << 16)
                  | (((acpi::srat_apic_affinity**)acpi_entries)[i]->hi_DM[2]
                     << 24);

            if ((domain + 1) > num_domains) { num_domains = (domain + 1); }
        }
        i++;
    }

    // Check x2apic entries
    num_entries
        = acpi::get_entries(srat, acpi::srat_entry_type::x2apic_affinity,
                            acpi_entries, num_entries);
    i = 0;
    while (i < num_entries) {
        if (((acpi::srat_x2apic_affinity**)acpi_entries)[i]->flags & 1) {

            unsigned int domain
                = ((acpi::srat_x2apic_affinity**)acpi_entries)[i]->domain;

            if ((domain + 1) > num_domains) { num_domains = (domain + 1); }
        }
        i++;
    }

    // Systems without NUMA domains have only 1 domain
    if (num_domains == 0) { num_domains = 1; }

    // Can create topology info on numa domains
    topology.num_domains = num_domains;
    topology.domains = (numa_domain*)malloc(sizeof(numa_domain) * num_domains);
    for (unsigned int j = 0; j < num_domains; j++) {
        topology.domains[j].num_socket = 0;
        topology.domains[j].start      = 0;
        topology.domains[j].end        = ~(0);
    }

    // Process memory entries and apply those to the numa domains
    num_entries = acpi::get_entries(srat, acpi::srat_entry_type::mem_affinity,
                                    acpi_entries, num_entries);
    i           = 0;
    while (i < num_entries) {
        if (((acpi::srat_mem_affinity**)acpi_entries)[i]->flags & 1) {
            topology
                .domains[((acpi::srat_mem_affinity**)acpi_entries)[i]->domain]
                .start
                = (((acpi::srat_mem_affinity**)acpi_entries)[i]->lo_base | 0ULL
                   | ((((acpi::srat_mem_affinity**)acpi_entries)[i]->hi_base
                       | 0ULL)
                      << 32));

            topology
                .domains[((acpi::srat_mem_affinity**)acpi_entries)[i]->domain]
                .end
                = (topology
                       .domains[((acpi::srat_mem_affinity**)acpi_entries)[i]
                                    ->domain]
                       .start
                   + (((acpi::srat_mem_affinity**)acpi_entries)[i]->lo_length
                      | 0ULL
                      | ((((acpi::srat_mem_affinity**)acpi_entries)[i]
                              ->hi_length
                          | 0ULL)
                         << 32)));
        }

        i++;
    }

    // Detection of layout of sockets and physical/logical cores
    // is different between AMD and Intel CPUs (other vendors are
    // assumed to follow Intel for simplicity's sake)
    unsigned int id_logical_bits     = 0;
    unsigned int id_physical_bits    = 0;
    unsigned int max_supported_cpuid = __get_cpuid_max(0U, (unsigned int*)0);
    unsigned int max_supported_ext   = __get_cpuid_max(0x80000000, 0);

    // Check the HTT flag (if false, then its one core, one thread)
    unsigned int eax, ebx = 0, ecx = 0, edx = 0;
    eax = 1;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    if ((edx & (1 << 28)) || num_logical > 1) {
        unsigned int thread_count = ((ebx & (0x00ff0000)) >> 16);

        if (std_k::get_vendor() == std_k::amd) {
            // AMD Procedure

            // If supported, use cpuid 0x80000008
            if (max_supported_ext >= 0x80000008) {
                eax = 0x80000008;
                __get_cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
                if (ecx & (0xf000)) {
                    id_physical_bits = (ecx & 0xf000) >> 12;
                } else {
                    id_physical_bits = std_k::round_next_binary_power(
                        (unsigned int)(ecx & 0xff));
                }

                id_logical_bits = std_k::round_next_binary_power(
                    thread_count >> id_physical_bits);
            } else {
                // Otherwise revert to cpuid 0x01 results
                id_logical_bits  = 0;
                id_physical_bits = std_k::round_next_binary_power(thread_count);
            }

        } else {
            // Intel Procedure

            // If supported, use cpuid 0x0b
            if (max_supported_cpuid >= 0x0b) {
                ecx = 0;
                eax = 0x0b;
                __get_cpuid_count(0x0b, 0, &eax, &ebx, &ecx, &edx);
                id_logical_bits = eax & 0x1f;
                ecx             = 1;
                eax             = 0x0b;
                __get_cpuid_count(0x0b, 1, &eax, &ebx, &ecx, &edx);
                id_physical_bits = (eax & 0x1f) - id_logical_bits;
            } else if (max_supported_cpuid >= 0x04) {
                eax = 0x04;
                __get_cpuid(0x04, &eax, &ebx, &ecx, &edx);
                unsigned int core_count = (eax >> 26) & 0x3f;
                unsigned int count      = 0;
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
                id_logical_bits = std_k::round_next_binary_power(thread_count);
            }
        }
    }

    // Final sorting of logical cores
    std_k::insertion_sort<logical_core>(logical_cores, num_logical);

    apic_id::thread_bits = id_logical_bits;
    apic_id::core_bits   = id_physical_bits;

    topology.num_logical  = num_logical;
    topology.threads      = logical_cores;
    topology.num_physical = num_physical;
    topology.num_sockets  = num_sockets;
}

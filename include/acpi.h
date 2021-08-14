#ifndef ACPI_H
#define ACPI_H

#include <stdint.h>

#include "libk.h"
#include "pintos_std.h"

namespace acpi {

    // RSDP Signature = char[8] = "RSD PTR "
    const uint64_t RSDP_SIGNATURE = 0x2052545020445352;

    enum table_signature : uint32_t {
        MADT = 0x43495041, // "APIC"    
        BGRT = 0x54524742, // "BGRT"
        BERT = 0x54524542, // "BERT"
        CPEP = 0x50455043, // "CPEP"
        DSDT = 0x54445344, // "DSDT"
        ECDT = 0x54444345, // "ECDT"
        EINJ = 0x4a4e4945, // "EINJ"
        ERST = 0x54535245, // "ERST"
        FADT = 0x50434146, // "FACP"
        FACS = 0x53434146, // "FACS"
        HEST = 0x54434548, // "HEST"
        MSCT = 0x5443534d, // "MSCT"
        MPST = 0x5453604d, // "MPST"
        OEM  = 0x004d454f, // "OEM/0"
        PMTT = 0x54544d50, // "PMTT"
        PSDT = 0x54445350, // "PSDT"
        RASF = 0x46534152, // "RASF"
        RSDT = 0x54445352, // "RSDT"
        SBST = 0x54534253, // "SBST"
        SLIT = 0x54494d53, // "SLIT"
        SRAT = 0x53415253, // "SRAT"
        SSDT = 0x54445353, // "SSDT"
        XSDT = 0x54445358, // "XSDT"
        HPET = 0x54455048, // "HPET"
    };

    enum table_entry_offset : size_t {
        MADT_offset = 8,
        SRAT_offset = 12,
        HPET_offset = 4,
    };

    enum madt_entry_type : uint8_t {
        processor_apic,
        io_apic,
        io_apic_src_override,
        io_apic_nmi_src,
        apic_nmi,
        apic_addr_override,
        processor_x2apic = 9,
    };

    enum srat_entry_type : uint8_t {
        apic_affinity,
        mem_affinity,
        x2apic_affinity,
    };

    struct entry_header {
        uint8_t type;
        uint8_t length;

        entry_header* next_entry() const {
            return (entry_header*)((uintptr_t)this + length);   
        }
    } __attribute__ ((packed));

    struct table_header {
        union {char c_signature[4]; uint32_t u_signature;};
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        char oem_id[6];
        char oem_table_id[8];
        uint32_t oem_revision;
        uint32_t creator_id;
        uint32_t creator_revision;

        bool valid_checksum() const {
            unsigned char sum = 0;
 
            for (uint32_t i = 0; i < length; i++)
            {
                sum += ((char*)this)[i];
            }

            return sum == 0;
        }

        entry_header* get_entry_start() const {
            size_t offset = 0;
            
            switch (u_signature) {

                case table_signature::MADT:
                    offset = table_entry_offset::MADT_offset;
                    break;
                
                case table_signature::SRAT:
                    offset = table_entry_offset::SRAT_offset;
                    break;

                default:
                    break;
            }

            return (entry_header*)((uintptr_t)this + sizeof(table_header) + offset);
        }

    } __attribute__ ((packed));

    struct rsdt : table_header {
        uint32_t tables[];
    } __attribute__ ((packed));

    struct xsdt : table_header {
        table_header* tables[];
    } __attribute__ ((packed));

    struct old_rsdp {
        char signature[8];
        uint8_t checksum;
        char oem_id[6];
        uint8_t revision;
        uint32_t rsdt_addr;

        bool is_modern() const {
            if (revision >= 2) {
                return true;
            } else {
                return false;
            }
        }

        rsdt* get_rsdt() const {
            return (rsdt*)(rsdt_addr | 0ULL);
        }

    } __attribute__ ((packed));

    struct modern_rsdp  : old_rsdp {
        uint32_t length;
        xsdt* xsdt_addr;
        uint8_t extended_checksum;
        uint8_t reserved[3];

        xsdt* get_xsdt() const {
            return xsdt_addr;
        }

    } __attribute__ ((packed));

    struct madt_processor_apic : entry_header {
        uint8_t processor_id;
        uint8_t apic_id;
        uint32_t flags;
    } __attribute__ ((packed));

    struct madt_io_apic : entry_header {
        uint8_t io_apic_id;
        uint8_t reserved;
        uint32_t io_apic_addr;
        uint32_t global_int_base;
    } __attribute__ ((packed));

    struct madt_io_apic_src_override : entry_header {
        uint8_t bus_src;
        uint8_t irq_src;
        uint32_t global_int;
        uint16_t flags;
    } __attribute__ ((packed));

    struct madt_io_apic_nmi_src : entry_header {
        uint8_t nmi_src;
        uint8_t reserved;
        uint16_t flags;
        uint32_t global_int;
    } __attribute__ ((packed));

    struct madt_apic_nmi : entry_header {
        uint8_t processor_id;
        uint16_t flags;
        uint8_t lint;
    } __attribute__ ((packed));

    struct madt_apic_addr_override : entry_header {
        uint16_t reserved;
        uint64_t apic_addr;
    } __attribute__ ((packed));

    struct madt_processor_x2apic : entry_header {
        uint16_t reserved;
        uint32_t processor_id;
        uint32_t flags;
        uint32_t acpi_id;
    } __attribute__ ((packed));

    struct madt_table : table_header {
        uint32_t local_apic_addr;
        uint32_t flags;
        entry_header entries[];

    } __attribute__ ((packed));

    struct srat_apic_affinity : entry_header {
        uint8_t lo_DM;
        uint8_t APIC_ID;
        uint32_t flags;
        uint8_t SAPIC_EID;
        uint8_t hi_DM[3];
        uint32_t CDM;
    } __attribute__ ((packed));

    struct srat_mem_affinity : entry_header {
        uint32_t domain; 
        uint8_t reserved1[2]; 
        uint32_t lo_base;
        uint32_t hi_base; 
        uint32_t lo_length;   
        uint32_t hi_length; 
        uint8_t reserved2[4]; 
        uint32_t flags;
        uint8_t reserved3[8];
    } __attribute__ ((packed));

    struct srat_x2apic_affinity : entry_header {
        uint8_t type;
        uint8_t length;
        uint8_t reserved1[2]; 
        uint32_t domain;
        uint32_t x2APIC_ID;
        uint32_t flags; 
        uint32_t _CDM;
        uint8_t reserved2[4];
    } __attribute__ ((packed));

    struct srat_table : table_header {
        uint8_t reserved[12];
        entry_header entries[];

    } __attribute__ ((packed));

    struct hpet_address {
        uint8_t address_space_id;
        uint8_t register_bit_width;
        uint8_t register_bit_offset;
        uint8_t reserved;
        uint64_t address;
    } __attribute__ ((packed));

    struct hpet_table : table_header {
        uint8_t hardware_rev_id;
        uint8_t comparator_count:5;
        uint8_t counter_size:1;
        uint8_t reserved:1;
        uint8_t legacy_replacement:1;
        uint16_t pci_vendor_id;
        hpet_address address;
        uint8_t hpet_number;
        uint16_t minimum_tick;
        uint8_t page_protection;
    } __attribute__ ((packed));

    old_rsdp* find_rsdp(multiboot_boot_info* mb_info);

    table_header* get_table(old_rsdp* rsdp, table_signature target);

    int get_entries(table_header* target_table, uint8_t target_type, entry_header** entry_buffer, int buffer_max = 32);
    int count_entries(table_header* target_table, uint8_t target_type);


}

#endif
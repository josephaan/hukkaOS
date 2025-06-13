#ifndef __ACPI__H
#define __ACPI__H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <bootboot.h>

extern BOOTBOOT bootboot; 

// Structs

typedef struct {
    char signature[8];       // "RSD PTR "
    uint8_t checksum;        // Checksum
    char oem_id[6];          // OEM ID
    uint8_t revision;        // Revision
    uint32_t rsdt_address;   // 32-bit physical address of RSDT
    uint32_t length;         // Length of the table (for XSDT)
    uint64_t xsdt_address;   // 64-bit physical address of XSDT
    uint8_t extended_checksum; // Extended checksum
    uint8_t reserved[3];     // Reserved
}__attribute__ ((packed)) rsdp_descriptor_t;

typedef struct {
    char signature[4];       // "RSDT" or "XSDT"
    uint32_t length;         // Length of the table
    uint8_t revision;        // Revision
    uint8_t checksum;        // Checksum
    char oem_id[6];          // OEM ID
    char oem_table_id[8];    // OEM table ID
    uint32_t oem_revision;   // OEM revision
    uint32_t creator_id;     // Creator ID
    uint32_t creator_revision; // Creator revision
}__attribute__ ((packed)) acpi_sdt_header_t;


typedef struct {
    acpi_sdt_header_t header;
    uint32_t pointer_to_other_sdt[];
}__attribute__ ((packed)) rsdt_t;

typedef struct {
    acpi_sdt_header_t header;
    uint64_t pointer_to_other_sdt[];
}__attribute__ ((packed)) xsdt_t;

typedef struct {
    acpi_sdt_header_t header;
    uint32_t local_controller_address;   // Local Interrupt Controller Address , The 32-bit physical address at which each processor can access its local interrupt controller. aka apic_address
    uint32_t flags;
    uint8_t entries[];               
}__attribute__ ((packed)) madt_t;

typedef struct {
    uint8_t type;
    uint8_t length;
}__attribute__ ((packed)) madt_entry_header_t;

typedef struct {
    madt_entry_header_t header;
    uint8_t acpi_processor_id;
    uint8_t apic_id;
    uint32_t flags;
}__attribute__ ((packed)) madt_local_apic_t;

typedef struct {
    madt_entry_header_t header;
    uint8_t io_apic_id;
    uint8_t reserved;
    uint32_t io_apic_address;
    uint32_t global_system_interrupt_base;
}__attribute__ ((packed)) madt_io_apic_t;



void parse_acpi();

void print_sdt_type();

acpi_sdt_header_t* find_table(rsdt_t* rsdt, const char* signature);
acpi_sdt_header_t* find_version_1_table(const char* signature);
acpi_sdt_header_t* find_version_2_up_table(const char* signature);
bool doChecksum(acpi_sdt_header_t *tableHeader);
bool is_xsdt();
bool is_rsdt();



#endif // !__ACPI__H
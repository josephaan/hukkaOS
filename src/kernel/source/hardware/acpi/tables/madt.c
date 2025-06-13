#include <hardware/acpi/tables/madt.h>

// These variables are essential for the apic
static uint32_t apic_base_address   ;
static uint32_t ioapic_base_address ;
// Madt parsing

void parse_madt(madt_t * madt){
    uint8_t* entry = madt->entries;
    while (entry < (uint8_t*)madt + madt->header.length) {
        madt_entry_header_t* hdr = (madt_entry_header_t*)entry;
        apic_base_address = madt->local_controller_address;
        switch (hdr->type) {
            case 0: // Local APIC
            {
                // Process local APIC entry
                //madt_local_apic_t* lapic = (madt_local_apic_t*)entry;
                break;
            }
            case 1: // I/O APIC
            {
                madt_io_apic_t* ioapic = (madt_io_apic_t*)entry;
                // Process I/O APIC entry
                ioapic_base_address = ioapic->io_apic_address;
                break;
            }
            // Handle other MADT entry types...
        }
        entry += hdr->length;
    }
}

uint32_t get_lapic_address()   { return apic_base_address;   }
uint32_t get_ioapic_addresss() { return ioapic_base_address; }
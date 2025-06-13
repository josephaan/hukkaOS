#include <hardware/acpi/acpi.h>
#include <hardware/apic/apic.h>
#include <hardware/apic/ioapic.h>
#include <hardware/acpi/tables/madt.h>
#include <memory/paging.h>
#include <debug/log.h>

// IOAPIC register offsets
#define IO_APIC_REG_SELECT 0x00
#define IO_APIC_REG_WINDOW 0x10
#define IO_APIC_REDTBL     0x10

static uint64_t ioapic_base_address;

// Function to write to an IOAPIC register
void ioapic_write(uint8_t reg, uint32_t value) {
    uint32_t volatile* ioapic = (uint32_t volatile*) (uintptr_t) ioapic_base_address;
    ioapic[0] = (reg & 0xFF);
    ioapic[4] = value;
}

// Function to read from an IOAPIC register
uint32_t ioapic_read(uint8_t reg) {
    uint32_t volatile* ioapic = (uint32_t volatile*) (uintptr_t) ioapic_base_address;
    ioapic[0] = (reg & 0xFF);
    return ioapic[4];
}


void ioapic_set_irq(uint8_t irq, uint8_t vector, uint8_t apic_id) {
    uint32_t low_index = 0x10 + 2 * irq;
    uint32_t high_index = low_index + 1;

    uint32_t low_value = vector | (0 << 8) | (0 << 11) | (0 << 13) | (0 << 15) | (0 << 16);
    
    // Write the high 32 bits (destination APIC ID)
    ioapic_write(high_index, apic_id << 24);

    // Write the low 32 bits (interrupt vector and flags)
    ioapic_write(low_index, low_value);

}

// Masks ioapic vectors
static void mask_all_ioapic_vectors() {
    // Read the I/O APIC version register to determine the number of redirection entries
    uint32_t version = ioapic_read(0x01); // Register 0x01 is the I/O APIC version register
    uint8_t max_redirection_entry = (version >> 16) & 0xFF;

    // Iterate over each redirection entry and mask it
    for (uint8_t i = 0; i <= max_redirection_entry; ++i) {
        uint32_t redtbl_low = 0x10 + 2 * i; // Low 32 bits of the redirection table entry
        uint32_t value = ioapic_read(redtbl_low);
        value |= (1 << 16); // Set the mask bit (bit 16)
        ioapic_write(redtbl_low, value);
    }
}

void ioapic_unmask_irq(uint8_t irq) {
    uint32_t ioredtbl = IO_APIC_REDTBL + 2 * irq;
    uint32_t value = ioapic_read(ioredtbl);
    
    // Clear the mask bit (bit 16)
    value &= ~(1 << 16);
    
    ioapic_write(ioredtbl, value);
}


// Kernel initialisation function
void ioapic_init(){
    //log(LOG_LEVEL_INFO, "Setting up I/O APIC");
    ioapic_base_address = get_ioapic_addresss();
    map_4kb_page(0xFFFFFFFFF7002000, ioapic_base_address, PTE_MMIO_FLAGS);
    ioapic_base_address = 0xFFFFFFFFF7002000;
    //log(LOG_LEVEL_INFO, "I/O APIC address = %p", ioapic_base_address);
    mask_all_ioapic_vectors();
    //log(LOG_LEVEL_INFO, "Masked All I/O APIC Vectors");

}

void ioapic_send_eoi(uint8_t vector) {
    *((volatile uint32_t *)(ioapic_base_address + 0xFEE000B0)) = vector;
}
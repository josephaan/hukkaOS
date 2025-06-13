#include <hardware/acpi/acpi.h>
#include <hardware/apic/apic.h>
#include <hardware/acpi/tables/madt.h>
#include <debug/log.h>
#include <memory/paging.h>

static uint64_t apic_base_address   ;

// IO function


static inline void outb(uint16_t port, uint8_t data) {
    //__asm__ __volatile__ ("outb %0, %1" : : "a"(data), "Nd"(port));
    __asm__ __volatile__ ("outb %%al, %%dx" :: "a" (data),"d" (port));
    
}

static uint8_t inb(uint16_t port) {
    unsigned char data = 0;
    //__asm__ __volatile__ ("inb %1, %0" : "=a"(data) : "Nd"(port));
    __asm__ __volatile__ ("inb %%dx, %%al" : "=a" (data) : "d" (port));
    return data;
}

// Reads an MSR(Model Specific Register)
static inline uint64_t read_msr(uint32_t msr) {
    uint32_t low, high;
    __asm__ __volatile__ ("rdmsr" : "=a" (low), "=d" (high) : "c" (msr));
    return ((uint64_t)high << 32) | low ;
}

// Write to an MSR (Model Specific Register)
static inline void write_msr(uint32_t msr, uint64_t value) {
    uint32_t low = (uint32_t)value;
    uint32_t high = (uint32_t)value;
    __asm__ __volatile__ ("wrmsr" :: "c" (msr), "a" (low), "d" (high));

}

static void enable_apic() {
    uint64_t msr = read_msr(IA32_APIC_BASE_MSR);
    msr |= APIC_BASE_MSR_ENABLE;                // Set the 11th bit
    write_msr(IA32_APIC_BASE_MSR, msr);
}

uintptr_t get_msr_apic_base_address() {
    uint64_t msr_value = read_msr(IA32_APIC_BASE_MSR);
    return (uintptr_t)(msr_value & 0xFFFFF000);
}

bool is_apic_enabled_in_msr() {
    uint64_t msr_apic_base_address = read_msr(IA32_APIC_BASE_MSR);
    return (msr_apic_base_address & (1 << 11)) != 0;          // Bit 11 is the global enable or disable flag
}


// Functions for reading and writing to apic_registers

// Function to write to an APIC register
void write_apic_register(uint32_t reg, uint32_t value) {
    *(uint32_t volatile*)(apic_base_address + reg) = value;
}

// Function to read from an APIC register
uint32_t read_apic_register(uint32_t reg) {
    return *(uint32_t volatile*)(apic_base_address + reg);
}


// Function to send EOI (End of Interrupt) to Local APIC
void send_eoi() {
    *(volatile uint32_t *)(apic_base_address + 0xB0) = 0;
}

// Kernel initialisation function
void apic_init() {
    log(LOG_LEVEL_INFO, "Initializing APIC");
    apic_base_address =  get_lapic_address() ;
    map_4kb_page(0xFFFFFFFFF7001000, apic_base_address, PTE_MMIO_FLAGS);

    log(LOG_LEVEL_INFO, "APIC address = %p", apic_base_address);
    uint32_t msr_apic_base_address = get_msr_apic_base_address();
    if (msr_apic_base_address != apic_base_address){
        log(LOG_LEVEL_WARN, "The MSR %p and MADT %p, APIC address are different", msr_apic_base_address, apic_base_address);
    }
    
    apic_base_address = 0xFFFFFFFFF7001000;

    bool apic_status = is_apic_enabled_in_msr();
    if(apic_status == true){
        log(LOG_LEVEL_INFO, "APIC ENABLED") ; 
    } else { 
        log(LOG_LEVEL_WARN, "APIC not enable, ... Enabling APIC");
        enable_apic();
    }
        /* Set the Spurious Interrupt Vector Register bit 8 to start receiving interrupts */
    write_apic_register(0xF0, read_apic_register(0xF0) | 0x100);
    log(LOG_LEVEL_INFO, "APIC Initialized");

}


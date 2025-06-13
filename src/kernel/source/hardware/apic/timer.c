#include <hardware/acpi/acpi.h>
#include <hardware/apic/apic.h>
#include <hardware/apic/ioapic.h>
#include <hardware/acpi/tables/madt.h>
#include <debug/log.h>
#include <display/framebuffer.h>
#include <x86_64/cpu.h>


// APIC TIMER SECTION
#define APIC_TIMER_PERIODIC 0x20000
#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_ENABLE 0x800

static uint32_t apic_base_address;
static uint64_t apic_ticks = 0;
static uint64_t sleep_count_down = 0;

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

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(data), "Nd"(port));
}

static uint8_t inb(uint16_t port) {
    unsigned char data = 0;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

uint32_t calibrate_apic_timer(void) {
    // Set up PIT to generate an interrupt after 1ms
    outb(0x43, 0x30);
    outb(0x40, 0xA9);
    outb(0x40, 0x04);

    // Set APIC timer to countdown mode
    write_apic_register(DIVIDE_CONFIGURATION_REGISTER_FOR_TIMER, 0x3);
    write_apic_register(INITIAL_COUNT_REGISTER_FOR_TIMER, 0xFFFFFFFF);

    // Wait for PIT interrupt
    while (!(inb(0x61) & 0x20));

    // Stop APIC timer
    write_apic_register(LVT_TIMER_REGISTER, APIC_TIMER_FLAG_MASKED);

    // Read the APIC timer value
    uint32_t apic_ticks = 0xFFFFFFFF - read_apic_register(CURRENT_COUNT_REGISTER_FOR_TIMER);

    return apic_ticks * 1000; // Return ticks per second
}

/*
void init_apic_timer(uint32_t frequency) {
    // Enable APIC if it's not already enabled
    apic_base_address = get_lapic_address();
    //write_msr(IA32_APIC_BASE_MSR, apic_base_address | IA32_APIC_BASE_MSR_ENABLE);

    // Set the timer divisor
    write_apic_register(DIVIDE_CONFIGURATION_REGISTER_FOR_TIMER, 0x3);

    // Calculate and set the initial count
    uint32_t apic_frequency = get_tsc_frequency() ; // determine_apic_frequency(); 
    
    if (apic_frequency == 0){
        apic_frequency = 100000; // Failsafe if get_tsc_frequency returns 0;
    }
    
    uint32_t initial_count = 1000000000 / frequency;
    write_apic_register(INITIAL_COUNT_REGISTER_FOR_TIMER, initial_count);

    // Configure LVT Timer in periodic mode
    write_apic_register(LVT_TIMER_REGISTER, APIC_TIMER_FLAG_PERIODIC | 34); // Assuming IRQ 0 is mapped to vector 34
}*/

void init_apic_timer(uint32_t frequency) {
    uint32_t apic_frequency = calibrate_apic_timer();
    if (apic_frequency == 0) {
        log(LOG_LEVEL_ERROR, "Failed to calibrate APIC timer");
        return;
    }

    uint32_t initial_count = apic_frequency / frequency;
    write_apic_register(DIVIDE_CONFIGURATION_REGISTER_FOR_TIMER, 0x3);
    write_apic_register(INITIAL_COUNT_REGISTER_FOR_TIMER, initial_count);
    write_apic_register(LVT_TIMER_REGISTER, (APIC_TIMER_FLAG_PERIODIC | 34));

    log(LOG_LEVEL_INFO, "APIC Timer initialized with frequency %lu Hz", frequency);
}


void init_apic_timer_interrupt() {
    // Register the interrupt handler
    init_apic_timer(10000);
    ioapic_set_irq(0, 0x22, 0);
    ioapic_unmask_irq(0);
    //log(LOG_LEVEL_DEBUG, "IRQ 0 : IDT 34 APIC Timer set");
}

/*
void sleep(uint64_t miliseconds){
    sleep_count_down = miliseconds;
    while (sleep_count_down > 0) {
        asm ("hlt");
    }
}*/

void sleep(uint64_t milliseconds) {
    uint64_t target_tick = apic_ticks + milliseconds;
    while (apic_ticks < target_tick) {
        asm volatile ("sti; hlt; cli");
    }
}

/*
void apic_timer_interrupt_handler() {
    // Handle APIC timer interrupt
    // This could also update a system tick count or perform other periodic tasks
    apic_ticks++;
    if(sleep_count_down > 0) sleep_count_down--;
    //print_numstr("The APIC Tick = %ld", 0xffff, apic_ticks);
    //log(LOG_LEVEL_INFO, "APIC TIMER TICKED");
    // Send EOI to the APIC
    //ioapic_send_eoi(0x22);
    send_eoi();
    // Function to send EOI (End of Interrupt) to Local APIC
}
*/

void apic_timer_interrupt_handler() {
    static uint64_t last_refresh = 0;
    const uint64_t refresh_interval = 16; // ~60 Hz refresh rate

    apic_ticks++;
    if (sleep_count_down > 0) sleep_count_down--;

    if (apic_ticks - last_refresh >= refresh_interval) {
        last_refresh = apic_ticks;
        //refresh_display(); // Implement this function to update the display
    }

    send_eoi();
}
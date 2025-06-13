#include <process/syscall/syscall.h>

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



//cpu_status_t* system_call_handler(cpu_status_t *registers){}

int sys_write(uint64_t dev_id, const void* buffer, size_t size) {
    /*OutputDevice* dev = get_output_device(dev_id);
    if (!dev) {
        return -1;  // Invalid device ID
    }
    return dev->write(dev, buffer, size);*/
}

void syscall_handler(cpu_status_t *registers){
    uint64_t syscall_number = registers->rdi;

    switch (syscall_number) {
        case SYSCALL_WRITE:
            // Implement write syscall
            uint64_t dev_id = registers->rsi;
            uint64_t buffer = registers->rdx;
            uint64_t size = registers->rcx;
            sys_write(dev_id, (const void*)buffer, size);
            break;
        case SYSCALL_READ:
            // Implement read syscall
            break;
        case SYSCALL_OPEN:
            // Implement open syscall
            break;
        case SYSCALL_CLOSE:
            // Implement close syscall
            break;
        default:
            // Handle unknown syscall
            break;
    }
}

void setup_syscall() {
    // Enable SYSCALL/SYSRET
    uint64_t efer = read_msr(0xC0000080);
    efer |= 1 << 0;  // Set SCE (System Call Enable) bit
    write_msr(0xC0000080, efer);

    // Set up STAR MSR
    // Bits 63:48 -> SYSRET CS and SS (63:48 = 0x1B, 47:32 = 0x23)
    // Bits 47:32 -> SYSCALL CS and SS (63:48 = 0x08, 47:32 = 0x10)
    uint64_t star = (0x23ULL << 48) | (0x08ULL << 32);
    write_msr(0xC0000081, star);

    // Set up LSTAR MSR (syscall entry point)
    write_msr(0xC0000082, (uint64_t)syscall_handler);

    // Set up FMASK MSR (FLAGS mask for syscall)
    // Disable interrupts, trap flag, and alignment check
    write_msr(0xC0000084, 0x257);
}



void syscall_write_handler(cpu_status_t * context){
    
}
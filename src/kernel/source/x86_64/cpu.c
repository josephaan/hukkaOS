#include <x86_64/cpu.h>
#include <display/framebuffer.h>
#include <string.h>

static uint32_t cpu_model = 0;
static uint32_t cpu_famiy = 0;
static char cpu_model_name[60] = {0}; // Never < 48
static char cpu_manufacturer[60] = {0};

typedef struct{
    uint32_t cpu_model;
    uint32_t cpu_famiy;
    uint32_t model;
    char cpu_model_name[60]; // Never < 48
    char cpu_manufacturer[60];
} cpu_info_t;

// Function to get the current core ID
uint32_t get_current_core_id(){
    uint32_t ebx;
    asm volatile(
        "cpuid"
        : "=b" (ebx)    // output in ebx
        : "a" (1)       // input eax=1
        : "ecx", "edx"  // clobbered registers
    );
    return ebx >> 24; // Local APIC ID is the upper 9 bits of ebx
}

struct cpuid_result {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
};

static inline struct cpuid_result cpuid(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    struct cpuid_result result;
    asm volatile("cpuid"
                 : "=a" (result.eax),
                   "=b" (result.ebx),
                   "=c" (result.ecx),
                   "=d" (result.edx)
                 : "0" (eax), "1" (ebx), "2" (ecx), "3" (edx)
                 : "memory");
    return result;
}

void detect_cpu(void) {
    struct cpuid_result vendor = cpuid(0, 0, 0, 0);
    char vendor_str[13];
    *(uint32_t*)(vendor_str + 0) = vendor.ebx;
    *(uint32_t*)(vendor_str + 4) = vendor.edx;
    *(uint32_t*)(vendor_str + 8) = vendor.ecx;
    vendor_str[12] = '\0';

    struct cpuid_result info = cpuid(1, 0, 0, 0);
    uint32_t family = (info.eax >> 8) & 0xF;
    cpu_famiy = family;
    uint32_t model = (info.eax >> 4) & 0xF;
    cpu_model = model;
    uint32_t stepping = info.eax & 0xF;

    // Print or store this information
    print_numstr("CPU Vendor: %s\n", 0xffff, vendor_str);
    print_numstr("CPU Family: %d, Model: %d, Stepping: %d\n",  0xffff, family, model, stepping);


    // Check and enable features
    if (info.edx & (1 << 25)) {
        print_numstr("SSE supported\n", 0xffff);
        // Enable SSE in CR4 if needed
    }
    if (info.ecx & (1 << 28)) {
        print_numstr("AVX supported\n", 0xffff);
        // Enable AVX if needed
    }

    
}

bool is_cpuid_leaf_supported(uint32_t leaf) {
    struct cpuid_result result = cpuid(0, 0, 0, 0);
    uint32_t max_leaf = result.eax;
    return leaf <= max_leaf;
}


// Function to read TSC
uint64_t read_tsc(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

uint64_t estimate_tsc_freq(void) {
    struct cpuid_result result = cpuid(0x1, 0, 0, 0);
    
    // Get base clock frequency (in MHz)
    uint32_t base_freq = (result.ebx >> 8) & 0xFF;
    
    // If base frequency is available, use it as a starting point
    if (base_freq != 0) {
        return (uint64_t)base_freq * 1000000; // Convert MHz to Hz
    }

    return 0;
    
    /*
    // If base frequency is not available, measure TSC
    struct timespec start, end;
    uint64_t tsc_start, tsc_end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    tsc_start = read_tsc();
    
    // Sleep for 10 ms
    struct timespec sleep_time = {0, 10000000};
    nanosleep(&sleep_time, NULL);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    tsc_end = read_tsc();
    
    uint64_t tsc_diff = tsc_end - tsc_start;
    uint64_t ns_diff = (end.tv_sec - start.tv_sec) * 1000000000ULL + (end.tv_nsec - start.tv_nsec);
    
    // Calculate frequency: cycles per second
    return (tsc_diff * 1000000000ULL) / ns_diff;
    */
}

uint64_t get_tsc_frequency(){
    struct cpuid_result tsc_data = cpuid(0x15, 0, 0, 0);
    /*
        If EBX[31:0] is 0, the TSC/”core crystal clock” ratio is not enumerated.
            EBX[31:0]/EAX[31:0] indicates the ratio of the TSC frequency and the core crystal clock frequency.
            If ECX is 0, the nominal core crystal clock frequency is not enumerated.
            “TSC frequency” = “core crystal clock frequency” * EBX/EAX.
        The core crystal clock may differ from the reference clock, bus clock, or core clock frequencies.
        EAX Bits 31-00: An unsigned integer which is the denominator of the TSC/”core crystal clock” ratio.
        EBX Bits 31-00: An unsigned integer which is the numerator of the TSC/”core crystal clock” ratio.
        ECX Bits 31-00: An unsigned integer which is the nominal frequency of the core crystal clock in Hz.
        EDX Bits 31-00: Reserved = 0
    */

   if(tsc_data.ecx != 0){ return tsc_data.ecx; }
   if(tsc_data.eax != 0 && tsc_data.ebx != 0 ) {
    uint32_t tsc_frequency = tsc_data.eax * tsc_data.ebx ;
    return tsc_frequency;
   } if (estimate_tsc_freq() != 0) {
        return estimate_tsc_freq();
   }
   return 0;
}

// Halt and catch fire function.
void hcf(void) {
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}

void disable_interrupts(void) {
    __asm__ volatile("cli" ::: "memory");
}

void enable_interrupts(void) {
    __asm__ volatile("sti" ::: "memory");
}

bool are_interrupts_enabled(void) {
    unsigned long flags;
    __asm__ volatile ( "pushf\n\t"
                       "pop %0"
                       : "=g"(flags) );
    return flags & (1 << 9);  // bit 9 is the interrupt flag
}

static inline void huka_cpuid(uint32_t function, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    asm volatile("cpuid"
                 : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
                 : "a" (function), "c" (0));
}

void identify_cpu() {
    uint32_t eax, ebx, ecx, edx;
    char vendor[13];
    char brand[49];

    enum color Color = Yellow;

    // Get vendor ID
    huka_cpuid(0, &eax, &ebx, &ecx, &edx);
    memcpy(vendor, &ebx, 4);
    memcpy(vendor + 4, &edx, 4);
    memcpy(vendor + 8, &ecx, 4);
    vendor[12] = '\0';
    print_numstr("CPU Vendor: %s\n", Color,  vendor);

    // Get model, family, and stepping
    huka_cpuid(1, &eax, &ebx, &ecx, &edx);
    uint32_t stepping = eax & 0xF;
    uint32_t model = (eax >> 4) & 0xF;
    uint32_t family = (eax >> 8) & 0xF;
    uint32_t extended_model = (eax >> 16) & 0xF;
    uint32_t extended_family = (eax >> 20) & 0xFF;

    if (family == 0xF) {
        family += extended_family;
    }
    if (family == 0x6 || family == 0xF) {
        model += (extended_model << 4);
    }

    print_numstr("CPU Family: %d, Model: %d, Stepping: %d\n",  Color, family, model, stepping);

    // Check if brand string is supported
    huka_cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    if (eax >= 0x80000004) {
        // Get brand string
        uint32_t brand_registers[12];
        for (int i = 0; i < 3; i++) {
            huka_cpuid(0x80000002 + i, &brand_registers[i*4], &brand_registers[i*4+1], &brand_registers[i*4+2], &brand_registers[i*4+3]);
        }
        memcpy(brand, brand_registers, sizeof(brand) - 1);
        memcpy(cpu_model_name, brand, sizeof(cpu_model) - 1);
        cpu_model_name[59] = '\0';
        brand[48] = '\0';
        
        // Trim leading spaces
        char *brand_trimmed = brand;
        while (*brand_trimmed == ' ') brand_trimmed++;
        
        print_numstr("CPU Brand: %s\n", Color, brand_trimmed);
    } else {
        print_numstr("CPU Brand: Not supported\n", Color);
    }

    // Get features (as before)
    huka_cpuid(1, &eax, &ebx, &ecx, &edx);
    print_numstr("CPU Features: ",  Color);
    if (edx & (1 << 0)) print_numstr("FPU ",  Color);
    if (edx & (1 << 23)) print_numstr("MMX ", Color);
    if (edx & (1 << 25)) print_numstr("SSE ", Color);
    if (edx & (1 << 26)) print_numstr("SSE2 ",Color);
    if (ecx & (1 << 0)) print_numstr("SSE3 ", Color);
    // Add more feature checks as needed
    print_numstr("\n",  Color);

    // Get extended features (as before)
    huka_cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
    print_numstr("Extended Features: ",  Color);
    if (edx & (1 << 29)) print_numstr("64-bit ",  Color);
    if (edx & (1 << 11)) print_numstr("SYSCALL/SYSRET ", Color);
    // Add more extended feature checks as needed
    print_numstr("\n",  Color);
}


// Function to be executed in user mode
void user_mode_function() {
    // User mode code here
    while(1) {
        // Do something in user mode
    }
}


void switch_to_user_mode(void* user_stack) {
    //disable_interrupts();

    uint64_t * user_addr_p = (void*)0x0000000400000000;

    memcpy(user_addr_p, user_mode_function, sizeof(user_mode_function));
    
    // Set up the stack for iret
    struct {
        uint64_t rip;
        uint64_t cs;
        uint64_t eflags;
        uint64_t rsp;
        uint64_t ss;
    } __attribute__((packed)) args;

    args.cs = USER_CS;
    args.ss = USER_SS;
    args.eflags = 0x202;  // Interrupts enabled
    args.rip = (uint64_t)user_addr_p;
    args.rsp = (uint64_t)user_stack;  // Allocate and return user stack top

    
    // Load the user data segment
    asm volatile(
        "movq $0x23, %%rax\n"
        "movq %%rax, %%ds\n"
        "movq %%rax, %%es\n"
        "movq %%rax, %%fs\n"
        "movq %%rax, %%gs\n"

        // Set up the stack frame for iretq
        "movq %0, %%rsp\n"            // Load the user-mode stack pointer
        "pushq $0x23\n"              // SS: User data segment
        "pushq %0\n"                 // RSP: User-mode stack pointer
        "pushfq\n"                   // RFLAGS
        "popq %%rax\n"               // Get current RFLAGS
        "or $0x200, %%rax\n"         // Set IF flag (enable interrupts)
        "pushq %%rax\n"              // Push modified RFLAGS back
        "pushq $0x1B\n"              // CS: User code segment
        "pushq %1\n"                 // RIP: Entry point
        "iretq"
        :
        : "r"(args.rsp), "r"(args.rip)
        : "rax"

    );

    //enable_interrupts();
    
}


void cpu_information(){
    // This function prints
    return;
}

uint64_t get_rsp(){
    uint64_t rsp;
    asm volatile("mov %%rsp, %0" : "=r"(rsp));
    return rsp;
}
#ifndef __CPU__H
#define __CPU__H

#include <stdint.h>
#include <stdbool.h>

#define KERNEL_CS (0x08 | 0)
#define KERNEL_SS (0x10 | 0)
#define USER_CS   (0x18 | 3)
#define USER_SS   (0x20 | 3)

typedef struct {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rbp;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    
    uint64_t interrupt_number;
    uint64_t error_code;
    
    uint64_t rip;
    uint16_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint16_t ss;

    /*  uint16_t ds;
        uint16_t es;
        uint16_t fs;
        uint16_t ds;
        uint16_t gs

    //X87 FPU registers

    long double ST0;
    long double ST1;
    long double ST2;
    long double ST3;
    long double ST4;
    long double ST5;
    long double ST6;
    long double ST7;
    
    
    // MMO registers

    long double MM0;
    long double MM1;
    long double MM2;
    long double MM3;
    long double MM4;
    long double MM5;
    long double MM6;
    long double MM7;
    
    // XMM registers, SSE Registers

    long double XMM0;
    long double XMM1;
    long double XMM2;
    long double XMM3;
    long double XMM4;
    long double XMM5;
    long double XMM6;
    long double XMM7;
    long double XMM8;
    long double XMM9;
    long double XMM10;
    long double XMM11;
    long double XMM12;
    long double XMM13;
    long double XMM14;
    long double XMM15;

    uint16_t control_register;
    uint16_t status_register;
    uint16_t tag_register;
    
    
    */

} __attribute__((__packed__)) cpu_status_t ;


// Function prototypes
uint32_t get_current_core_id();
uint64_t get_tsc_frequency();
uint64_t estimate_tsc_freq(void);
void detect_cpu(void);
void identify_cpu();
bool is_cpuid_leaf_supported(uint32_t leaf);
void hcf(void);

// Usermode Function

void switch_to_user_mode(void* user_stack);


// Interrupt Function

void disable_interrupts(void);
void enable_interrupts(void);
bool are_interrupts_enabled(void);

uint64_t get_rsp();



#endif
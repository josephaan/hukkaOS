#include <x86_64/idt.h>
#include <debug/log.h>


void division_by_zero_handler() {
    log(LOG_LEVEL_DEBUG, "Division by Zero Exception\n");
    // Handle the exception or halt the CPU
    while(1);
}

void debug_exception_handler() {
    log(LOG_LEVEL_DEBUG, "Debug Exception\n");
    // Handle the exception or halt the CPU
    while(1);
}

void non_maskable_interrupt_handler() {
    log(LOG_LEVEL_DEBUG, "Non-Maskable Interrupt\n");
    // Handle the exception or halt the CPU
    while(1);
}

void breakpoint_handler() {
    log(LOG_LEVEL_DEBUG, "Breakpoint\n");
    // Handle the exception or halt the CPU
    while(1);
}

void overflow_handler() {
    log(LOG_LEVEL_DEBUG, "Overﬂow\n");
    // Handle the exception or halt the CPU
    while(1);
}

void bound_range_exceeded_handler() {
    log(LOG_LEVEL_DEBUG, "Bound Range Exceeded\n");
    // Handle the exception or halt the CPU
    while(1);
}

void invalid_opcode_handler() {
    log(LOG_LEVEL_DEBUG, "Invalid/undeﬁned opcode\n");
    // Handle the exception or halt the CPU
    while(1);
}

void no_math_coprocessor_handler() {
    log(LOG_LEVEL_DEBUG, "No Math Coprocessor\n");
    // Handle the exception or halt the CPU
    while(1);
}

//__attribute__((interrupt)) 
void double_fault_handler() {
    log(LOG_LEVEL_DEBUG, "Double Fault \n");
    // Handle the exception or halt the CPU
    while(1);
}

void coprocessor_segment_overrun_handler() {
    log(LOG_LEVEL_DEBUG, "Coprocessor Segment Overrun | Reserved");
    // Handle the exception or halt the CPU
    while(1);
}

void invalid_tss_handler() {
    log(LOG_LEVEL_DEBUG, "Invalid TSS Exception\n");
    // Handle the exception or halt the CPU
    while(1);
}

void segment_not_present_handler() {
    log(LOG_LEVEL_DEBUG, "Segment Not Present Interrupt\n");
    // Handle the exception or halt the CPU
    while(1);
}

void stack_segment_fault_handler() {
    log(LOG_LEVEL_DEBUG, "Stack-Segment Fault\n");
    // Handle the exception or halt the CPU
    while(1){
        asm("hlt");
    }
}

//__attribute__((interrupt)) 
void general_protection_fault_handler(uint64_t error_code) {
    log(LOG_LEVEL_DEBUG, "General Protection Fault\n");
    log(LOG_LEVEL_DEBUG, "The Error Code is %ld", error_code);
    // Handle the exception or halt the CPU
    while(1){
        asm("hlt");
    }
}

void isr15_handler() {
    log(LOG_LEVEL_DEBUG, "Reserved");
    // Handle the exception or halt the CPU
    while(1);
}

void x87_fpu_error_handler() {
    log(LOG_LEVEL_DEBUG, "x87 FPU Floating-Point Error\n");
    // Handle the exception or halt the CPU
    while(1);
}

void alignment_check_exception_handler() {
    log(LOG_LEVEL_DEBUG, "Alignment Check Exception\n");
    // Handle the exception or halt the CPU
    while(1);
}

void machine_check_exception_handler() {
    log(LOG_LEVEL_DEBUG, "Machine Check Exception\n");
    // Handle the exception or halt the CPU
    while(1);
}

void simd_floating_point_exception_handler(){
    log(LOG_LEVEL_DEBUG, "SIMD Floating POINT Exception\n");
    // Handle the exception or halt the CPU
    while(1);
}

void security_exception_handler(){
    log(LOG_LEVEL_DEBUG, "Security Exception\n");
    // Handle the exception or halt the CPU
    while(1){
        asm("hlt");
    }
}

void isrX_handler() {
    log(LOG_LEVEL_DEBUG, "Undefined Interrupt\n");
    // Handle the exception or halt the CPU
    while(1);
}
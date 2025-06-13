#ifndef __IDT__H
#define __IDT__H

#include <x86_64/cpu.h>

// assembler isr functions

extern void interrupt_service_routine_0();
extern void interrupt_service_routine_1();
extern void interrupt_service_routine_2();
extern void interrupt_service_routine_3();
extern void interrupt_service_routine_4();
extern void interrupt_service_routine_5();
extern void interrupt_service_routine_6();
extern void interrupt_service_routine_7();
extern void interrupt_service_routine_error_code_8();
extern void interrupt_service_routine_9();
extern void interrupt_service_routine_error_code_10();
extern void interrupt_service_routine_error_code_11();
extern void interrupt_service_routine_error_code_12();
extern void interrupt_service_routine_error_code_13();
extern void interrupt_service_routine_error_code_14();
extern void interrupt_service_routine_15();
extern void interrupt_service_routine_16();
extern void interrupt_service_routine_error_code_17();
extern void interrupt_service_routine_18();
extern void interrupt_service_routine_32();
extern void interrupt_service_routine_33();
extern void interrupt_service_routine_34();
extern void interrupt_service_routine_35();
extern void interrupt_service_routine_46();
extern void interrupt_service_routine_47();
extern void interrupt_service_routine_128();
extern void interrupt_service_routine_255();


/* Function defined in idt.c */

//void isr_handler(uint64_t);

void idt_init();

// Exception definition type

#define DIVIDE_BY_ZERO_ERROR 0x00    		// Interrupt 0
#define DEBUG_EXCEPTION 0x01				// Interrupt 1
#define NON_MASKABLE_INTERRUPT 0x02			// Interrupt 2
#define BREAKPOINT 0x03						// Interrupt 3
#define OVERFLOW 0x04						// Interrupt 4
#define BOUND_RANGE_EXCEEDED 0x05			// Interrupt 5
#define INVALID_OPCODE 0x06					// Interrupt 6
#define DEVICE_NOT_AVAILABLE 0x07			// Interrupt 7
#define DOUBLE_FAULT 0x08					// Interrupt 8
#define COPROCESSOR_SEGMENT_OVERRUN 0x09	// Interrupt 9
#define INVALID_TSS 0x0A					// Interrupt 10
#define SEGMENT_NOT_PRESENT 0x0B			// Interrupt 11
#define STACK_SEGMENT_FAULT 0x0C			// Interrupt 12
#define GENERAL_PROTECTION_FAULT 0x0D		// Interrupt 13
#define PAGE_FAULT 0x0E						// Interrupt 14
#define RESERVED 0x0F						// Interrupt 15
#define x87_FLOATING_POINT_EXCEPTION 0x10	// Interrupt 16
#define ALIGNMENT_CHECK 0x11				// Interrupt 17
#define MACHINE_CHECK 0x12					// Interrupt 18
#define SIMD_FLOATING_POINT_EXCEPTION 0x13	// Interrupt 19
#define RESERVED_0 0x14						// Interrupt 20
#define RESERVED_1 0x15						// Interrupt 21
#define RESERVED_2 0x16						// Interrupt 22
#define RESERVED_3 0x17						// Interrupt 23
#define RESERVED_4 0x18						// Interrupt 24
#define RESERVED_5 0x19						// Interrupt 25
#define RESERVED_6 0x1A						// Interrupt 26
#define RESERVED_7 0x1B						// Interrupt 27
#define RESERVED_8 0x1C						// Interrupt 28
#define RESERVED_9 0x1D						// Interrupt 29
#define SECURITY_EXCEPTION 0x1E				// Interrupt 30
#define RESERVED_10 0x1F					// Interrupt 31
#define PIT_TIMER_INTERRUPT 0x20			// Interrupt 32
#define KEYBOARD_IRQ 0x21				    // Interrupt 33
#define APIC_TIMER_IRQ 0x22			        // Interrupt 34
#define MOUSE_IRQ      0x23                 // Interrupt 35
#define ATA_PRIMARY_IRQ        0x2E         // Interrupt 46
#define ATA_SECONDARY_IRQ      0x2F         // Interrupt 47
#define SYSTEM_CALL_INTERRUPT 0x80          // Interrupt 128
#define APIC_SPURIOUS_INTERRUPT 0xFF		// Interrupt 255


// ISR handlers, most are defined in isr.c

void division_by_zero_handler();
void debug_exception_handler();
void non_maskable_interrupt_handler();
void breakpoint_handler();
void overflow_handler();
void bound_range_exceeded_handler();
void invalid_opcode_handler();
void no_math_coprocessor_handler();
void double_fault_handler();
void coprocessor_segment_overrun_handler();
void invalid_tss_handler();
void segment_not_present_handler();
void stack_segment_fault_handler();
void general_protection_fault_handler(uint64_t error_code);
void page_fault_handler(uint64_t error_code) ;
void isr15_handler() ;
void x87_fpu_error_handler() ;
void alignment_check_exception_handler() ;
void machine_check_exception_handler(); 
void simd_floating_point_exception_handler();
void security_exception_handler();
void isrX_handler() ;


#endif
#include <stdint.h>
#include <x86_64/idt.h>
#include <string.h>
#include <hardware/ps2/devices/keyboard.h>
#include <hardware/ps2/devices/mouse.h>
#include <memory/paging.h>
#include <hardware/storage/ata.h>
#include <process/syscall/syscall.h>


#define IDT_SIZE 256           			// defines the max size of idt entries
//#define KERNEL_CS   0x8       			// defines the kernel code segment
#define IDT_PRESENT_FLAG 0x80  			// defines the idt present flag
#define IDT_INTERRUPT_TYPE_FLAG 0x0E 

/// IDT entry structure for 64-bit mode
struct idt_entry {
    uint16_t base_lo;  // The lower 16 bits of the address to jump to when this interrupt fires.
    uint16_t sel;      // Kernel segment selector.
    uint8_t  always0;  // This must always be zero.
    uint8_t  flags;    // More flags. See documentation.
    uint16_t base_mid; // The next 16 bits of the address to jump to.
    uint32_t base_hi;  // The upper 32 bits of the address to jump to.
    uint32_t always0_2;// Reserved, set to zero.
} __attribute__((packed));

// IDT pointer structure
struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

// C-level interrupt handler function 

cpu_status_t* interrupts_handler(cpu_status_t *status){
    switch(status->interrupt_number){
        case DIVIDE_BY_ZERO_ERROR:
            division_by_zero_handler();
            break;
        case DEBUG_EXCEPTION:
            debug_exception_handler();
            break;
        case NON_MASKABLE_INTERRUPT:
            non_maskable_interrupt_handler();
            break;
        case BREAKPOINT:
            breakpoint_handler();
            break;
        case OVERFLOW:
            overflow_handler();
            break;
        case BOUND_RANGE_EXCEEDED:
            bound_range_exceeded_handler();
            break;
        case INVALID_OPCODE:
            invalid_opcode_handler();
            break;
        case DEVICE_NOT_AVAILABLE:
            no_math_coprocessor_handler();
            break;
        case DOUBLE_FAULT:
            double_fault_handler();
            break;
        case COPROCESSOR_SEGMENT_OVERRUN:
            coprocessor_segment_overrun_handler();
            break;
        case INVALID_TSS:
            invalid_tss_handler();
            break;
        case SEGMENT_NOT_PRESENT:
            segment_not_present_handler();
            break;
        case STACK_SEGMENT_FAULT:
            stack_segment_fault_handler();
            break;
        case GENERAL_PROTECTION_FAULT:
            general_protection_fault_handler(status->error_code);
            break;
        case PAGE_FAULT:
            page_fault_handler(status->error_code);
            break;
        case RESERVED:
            isr15_handler();
            break;
        case x87_FLOATING_POINT_EXCEPTION:
            x87_fpu_error_handler();
            break;
        case ALIGNMENT_CHECK:
            alignment_check_exception_handler();
            break;
        case MACHINE_CHECK:
            machine_check_exception_handler();
            break;
        case SIMD_FLOATING_POINT_EXCEPTION:
            simd_floating_point_exception_handler();
            break;
        case SECURITY_EXCEPTION:
            security_exception_handler();
            break;
        case PIT_TIMER_INTERRUPT:
            //pit_interrupt_handler();
            break;
        case KEYBOARD_IRQ:
            keyboard_interrupt_handler();
            break;
        case APIC_TIMER_IRQ:
            apic_timer_interrupt_handler();
            break;
        case MOUSE_IRQ:
            mouse_handler();
            break;
        case ATA_PRIMARY_IRQ:
            break;
        case ATA_SECONDARY_IRQ:
            break;
        case SYSTEM_CALL_INTERRUPT:
            //system_call_handler(status);
            break;
        default:
            isrX_handler();
            asm("hlt");
            break;
    }
    return status;
}

// Declare the IDT and IDT pointer
struct idt_entry idt[256];
struct idt_ptr idt_pointer;

void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_mid = (base >> 16) & 0xFFFF;
    idt[num].base_hi = (base >> 32) & 0xFFFFFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
    idt[num].always0_2 = 0;
}

void idt_init() {
    idt_pointer.limit = (sizeof(struct idt_entry) * 256) - 1;
    idt_pointer.base = (uint64_t)&idt;

    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    // Set IDT gates for the first two exceptions
    idt_set_gate(DIVIDE_BY_ZERO_ERROR,        (uint64_t)interrupt_service_routine_0, 0x08,  0x8E);
    idt_set_gate(DEBUG_EXCEPTION,             (uint64_t)interrupt_service_routine_1, 0x08,  0x8E);
    idt_set_gate(NON_MASKABLE_INTERRUPT,      (uint64_t)interrupt_service_routine_2, 0x08,  0x8E);
    idt_set_gate(BREAKPOINT,                  (uint64_t)interrupt_service_routine_3, 0x08,  0x8E);
    idt_set_gate(OVERFLOW,                    (uint64_t)interrupt_service_routine_4, 0x08,  0x8E);
    idt_set_gate(BOUND_RANGE_EXCEEDED,        (uint64_t)interrupt_service_routine_5, 0x08,  0x8E);
    idt_set_gate(INVALID_OPCODE,              (uint64_t)interrupt_service_routine_6, 0x08,  0x8E);
    idt_set_gate(DEVICE_NOT_AVAILABLE,        (uint64_t)interrupt_service_routine_7, 0x08,  0x8E);
    idt_set_gate(DOUBLE_FAULT,                (uint64_t)interrupt_service_routine_error_code_8, 0x08,  0x8E);
    idt_set_gate(COPROCESSOR_SEGMENT_OVERRUN, (uint64_t)interrupt_service_routine_9, 0x08,  0x8E);
    idt_set_gate(INVALID_TSS,                 (uint64_t)interrupt_service_routine_error_code_10, 0x08, 0x8E);
    idt_set_gate(SEGMENT_NOT_PRESENT,         (uint64_t)interrupt_service_routine_error_code_11, 0x08, 0x8E);
    idt_set_gate(STACK_SEGMENT_FAULT,         (uint64_t)interrupt_service_routine_error_code_12, 0x08, 0x8E);
    idt_set_gate(GENERAL_PROTECTION_FAULT,    (uint64_t)interrupt_service_routine_error_code_13, 0x08, 0x8E);
    idt_set_gate(PAGE_FAULT,                  (uint64_t)interrupt_service_routine_error_code_14, 0x08, 0x8E);
    idt_set_gate(RESERVED,                    (uint64_t)interrupt_service_routine_15, 0x08, 0x8E);
    idt_set_gate(x87_FLOATING_POINT_EXCEPTION,(uint64_t)interrupt_service_routine_16, 0x08, 0x8E);
    idt_set_gate(ALIGNMENT_CHECK,             (uint64_t)interrupt_service_routine_error_code_17, 0x08, 0x8E);
    idt_set_gate(MACHINE_CHECK,               (uint64_t)interrupt_service_routine_18, 0x08, 0x8E);

    // ..........
    idt_set_gate(PIT_TIMER_INTERRUPT,(uint64_t)interrupt_service_routine_32, 0x08, 0x8E);
    idt_set_gate(KEYBOARD_IRQ,(uint64_t)interrupt_service_routine_33, 0x08, 0x8E);
    idt_set_gate(APIC_TIMER_IRQ,(uint64_t)interrupt_service_routine_34, 0x08, 0x8E);
    // Set up the IDT entry for mouse interrupts
    idt_set_gate(MOUSE_IRQ,(uint64_t)interrupt_service_routine_35, 0x08, 0x8E);
    // ATA
    idt_set_gate(ATA_PRIMARY_IRQ,(uint64_t)interrupt_service_routine_46, 0x08, 0x8E);
    idt_set_gate(ATA_SECONDARY_IRQ,(uint64_t)interrupt_service_routine_47, 0x08, 0x8E);
    // SYSCALL
    idt_set_gate(SYSTEM_CALL_INTERRUPT,(uint64_t)interrupt_service_routine_128, 0x08, 0x8E);
    
 
    __asm__ __volatile__("lidt %0" : : "m"(idt_pointer));
    __asm__ __volatile__("sti");
    //log(LOG_LEVEL_INFO, "IDT set");
}
#include <stdint.h>
#include <x86_64/gdt.h>
#include <x86_64/cpu.h>

#define STACK_SIZE 2048

// TSS structure
struct tss_entry {
    uint32_t reserved1;
    uint64_t rsp0;        // Stack pointer for Ring 0
    uint64_t rsp1;        // Stack pointer for Ring 1
    uint64_t rsp2;        // Stack pointer for Ring 2
    uint64_t reserved2;
    uint64_t ist[7];      // Interrupt Stack Table
    uint64_t reserved3;
    uint16_t reserved4;
    uint16_t iopb_offset;
} __attribute__((packed));


// Segment Descriptor structure

struct gdt_entry{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;

}__attribute__((packed));

// Pointer to the GDT
struct gdtr {
    uint16_t limit;
    uint64_t base;

}__attribute__((packed));



struct gdt_entry gdt_n[7];
struct gdtr gdt_pointer;
struct tss_entry tss __attribute__((aligned(16)));

extern void gdt_flush();


// Function to set a GDT entry

void gdt_set_gate(uint16_t num, uint32_t base , uint32_t limit, uint8_t access,  uint8_t granularity){
    gdt_n[num].limit_low = 0;
    gdt_n[num].base_low = (base & 0xFFFF);
    gdt_n[num].base_middle = (base >> 16) & 0xFF;
    gdt_n[num].access = access;
    gdt_n[num].granularity = ((limit >> 16) & 0xFF) | (granularity & 0xF0);
    gdt_n[num].base_high = (base >> 24) & 0xFF; 
}


void init_gdt(){
    gdt_pointer.limit = (sizeof(struct gdt_entry)* 7) - 1;
    gdt_pointer.base  = (uint64_t)&gdt_n;

    // Null segment
    gdt_set_gate(0, 0, 0, 0, 0);

    // Kernel code segment
    gdt_set_gate(1, 0, 0, 0x9A, 0x20); // 0x20 for long mode

    // Kernel data segment
    gdt_set_gate(2, 0, 0, 0x92, 0);

    // User code segment
    gdt_set_gate(3, 0, 0, 0xFA, 0x20); // 0x20 for long mode

    // User data segment
    gdt_set_gate(4, 0, 0, 0xF2, 0);

    // TSS

    uint64_t tss_base = (uint64_t)&tss;
    uint32_t tss_limit = sizeof(struct tss_entry);

    gdt_set_gate(5, tss_base & 0xffffffff, tss_limit, 0x89, 0x40);
    
    // Set upper 32 bits of the TSS base address
    *(uint64_t*)((uint8_t*)&gdt_n[5] + 8) = tss_base >> 32;

    // Clear TSS
    for (int i = 0; i < sizeof(struct tss_entry); i++){
        ((char*)&tss)[i] = 0;
    }
    
    // Initialize TSS
    tss.rsp0 = get_rsp(); // Will be set when scheduling tasks
    tss.iopb_offset = tss_limit;

    //Load GDT and TSS

    // Load the GDT pointer
    gdt_flush();
    __asm__ __volatile__ ("ltr %%rax" : : "a" (0x28)); // 0x28 is the TSS segment selector

}


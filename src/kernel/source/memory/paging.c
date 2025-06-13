/*
 * paging.c
 * 
 *
 * Copyright (C) 2024 - 2026 Joseph Haita (haitajoseph2@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the HukaOS. 
 * 
 *
 */

#include <memory/paging.h>
#include <x86_64/cpu.h>
#include <memory/phys.h>
#include <memory/pmm.h>
#include <bootboot.h>
#include <terminal/terminal.h>
#include <debug/log.h>
#include <string.h>
#include <hardware/acpi/tables/madt.h>


#define KERNEL_START           0xFFFFFFFFF8000000
#define FRAMEBUFFER_START      0xFFFFFFFFFC000000
#define BOOTBOOT_STRUCTURE     0xFFFFFFFFFFE00000
#define ENVIRONMENT_CONFIG     0xFFFFFFFFFFE01000
#define KERENEL_CODE_AND_DATA  0xFFFFFFFFFFE02000

#define ENTRIES_PER_TABLE 512
#define HIGHER_HALF_START 256  // PML4 entry where higher half starts (usually 256)

extern BOOTBOOT bootboot;
extern uint64_t initstack;

static uint64_t * bootboot_paging = NULL;

// Reads CR3
static inline uint64_t read_cr3(void) {
    uint64_t cr3;
    __asm__ __volatile__("mov %%cr3, %0" : "=r" (cr3));
    return cr3;
}

// Writes to cr3
static inline void write_cr3(uint64_t cr3) {
    __asm__ __volatile__("mov %0, %%cr3" : : "r" (cr3) : "memory");
}

// Invalidate TLB entry
static inline void invlpg(void *address) {
    __asm__ __volatile__("invlpg (%0)" : : "r" (address) : "memory");
}

// Gets the physical addresss from a page table entry
static inline uint64_t get_physical_address(page_table_entry_t *pte) {
    return pte->entry &  0x000FFFFFFFFFF000ULL;
}

// Checks if a page table entry is present
static inline int is_present(page_table_entry_t *pte) {
    return pte->entry & 0x1;
}

static void keep_top_pml4_entry(void) {
    uint64_t cr3 = read_cr3();
    page_table_entry_t *pml4 = (page_table_entry_t *)(cr3 & ~0xFFFULL);

    // Keep only the first pml4 entry
    for (uint32_t i = 1; i < PAGE_ENTRIES; i++) {
        if (is_present(pml4)) {
            pml4[i].entry = 0;

            // Invalidate TLB for addresses using this entry
            invlpg((void *)(i * (1ULL << 39)));
        }
    }

    // Flush the TLB by reloading CR3
    write_cr3(cr3);
}

// This translate virtual address to physical address
uint64_t translate_virtual_to_physical(uint64_t virtual_address) {
    uint64_t cr3 = read_cr3();
    page_table_entry_t *pml4 = (page_table_entry_t *)(cr3 & ~0xFFFULL);
    
    // Extract page table indices
    uint64_t pml4_index = (virtual_address >> 39) & 0x1FF;
    uint64_t pdpt_index = (virtual_address >> 30) & 0x1FF;
    uint64_t pd_index = (virtual_address >> 21) & 0x1FF;
    uint64_t pt_index = (virtual_address >> 12) & 0x1FF;
    uint64_t page_offset = virtual_address & 0xFFF;

    // Traverse PML4
    if (!is_present(&pml4[pml4_index])) {
        return 0; // Address not mapped
    }
    page_table_entry_t *pdpt = (page_table_entry_t *)get_physical_address(&pml4[pml4_index]);

    // Traverse PDPT
    if (!is_present(&pdpt[pdpt_index])) {
        return 0; // Address not mapped
    }
    page_table_entry_t *pd = (page_table_entry_t *)get_physical_address(&pdpt[pdpt_index]);

    // Check if this is a 1GB page
    if (pdpt[pdpt_index].entry & (1ULL << 7)) {
        return (get_physical_address(&pdpt[pdpt_index]) & ~0x3FFFFFFF) | (virtual_address & 0x3FFFFFFF);
    }

    // Traverse PD
    if (!is_present(&pd[pd_index])) {
        return 0; // Address not mapped
    }
    page_table_entry_t *pt = (page_table_entry_t *)get_physical_address(&pd[pd_index]);

    // Check if this is a 2MB page
    if (pd[pd_index].entry & (1ULL << 7)) {
        return (get_physical_address(&pd[pd_index]) & ~0x1FFFFF) | (virtual_address & 0x1FFFFF);
    }

    // Traverse PT
    if (!is_present(&pt[pt_index])) {
        return 0; // Address not mapped
    }

    // Calculate the final physical address
    return get_physical_address(&pt[pt_index]) | page_offset;
}

void traverse_paging_structure(void) {
    uint64_t cr3 = read_cr3();
    page_table_entry_t *pml4 = (page_table_entry_t *)(cr3 & ~0xFFFULL);
    
    if (!is_present(&pml4[0])) {
        terminal_write("First PML4 entry is not present\n");
        return;
    }
    
    page_table_entry_t *pdpt = (page_table_entry_t *)get_physical_address(&pml4[0]);
    
    for (int i = 0; i < PAGE_ENTRIES; i++) {
        if (is_present(&pdpt[i])) {
            page_table_entry_t *pd = (page_table_entry_t *)get_physical_address(&pdpt[i]);
            
            for (int j = 0; j < PAGE_ENTRIES; j++) {
                if (is_present(&pd[j])) {
                    page_table_entry_t *pt = (page_table_entry_t *)get_physical_address(&pd[j]);
                    
                    for (int k = 0; k < PAGE_ENTRIES; k++) {
                        if (is_present(&pt[k])) {
                            uint64_t physical_address = get_physical_address(&pt[k]);
                            log(LOG_LEVEL_DEBUG,"Physical Address: %p\n", physical_address);
                        }
                    }
                }
            }
        }
    }
}

// Gets the index in the page table for a given level
static inline int get_table_index(uint64_t virtual_address, uint32_t level) {
    return (virtual_address >> (12 + 9 * (3 - level))) & 0x1FF;
}

// Creates or gets page table
static page_table_entry_t *create_or_get_table(page_table_entry_t *parent_entry) {
    if (!is_present(parent_entry)) {
        uint64_t new_table = (uint64_t) allocate_memory(PAGE_SIZE); //Allocate page needs to be implimented
        if(new_table == 0){
            log(LOG_LEVEL_ERROR, "No Memory Available for Page");
            return NULL;
        }
        parent_entry->entry = new_table | PAGE_PRESENT | PAGE_WRITABLE;
    }
    return (page_table_entry_t *)get_physical_address(parent_entry);
}

void map_4kb_page(uint64_t virt, uint64_t phys, uint64_t flags) {
    page_table_entry_t *pml4 = (page_table_entry_t*)(read_cr3() & ~0xFFF);

    uint64_t pml4_index = (virt >> 39) & 0x1FF;
    uint64_t pdpt_index = (virt >> 30) & 0x1FF;
    uint64_t pd_index = (virt >> 21) & 0x1FF;
    uint64_t pt_index = (virt >> 12) & 0x1FF;

    // Check if PML4 entry exists
    page_table_entry_t *pdpt = create_or_get_table(&pml4[pml4_index]);
    page_table_entry_t *pd = create_or_get_table(&pdpt[pdpt_index]);
    page_table_entry_t *pt = create_or_get_table(&pd[pd_index]);

    // Map the 4K page
    pt[pt_index].entry = phys | flags;

    return; // Success
}

void map_2mb_page(uint64_t virt, uint64_t phys, uint64_t flags) {
    page_table_entry_t *pml4 = (page_table_entry_t*)(read_cr3() & ~0xFFF);

    uint64_t pml4_index = (virt >> 39) & 0x1FF;
    uint64_t pdpt_index = (virt >> 30) & 0x1FF;
    uint64_t pd_index   = (virt >> 21) & 0x1FF;

        // Check if PML4 entry exists
    page_table_entry_t *pdpt = create_or_get_table(&pml4[pml4_index]);
    page_table_entry_t *pd = create_or_get_table(&pdpt[pdpt_index]);

    // Map the 2MB page
    pd[pd_index].entry = phys | flags | PAGE_LARGE;

    return ; // Success
}

// Function to unmap a page (works for both 4KB and 2MB pages)
void unmap_page(uint64_t virt)
{
    page_table_entry_t *pml4 = (page_table_entry_t*)(read_cr3() & ~0xFFF);

    uint64_t pml4_index = (virt >> 39) & 0x1FF;
    uint64_t pdpt_index = (virt >> 30) & 0x1FF;
    uint64_t pd_index   = (virt >> 21) & 0x1FF;
    uint64_t pt_index   = (virt >> 12) & 0x1FF;

    // Check if PML4 entry exists
    if (!(pml4[pml4_index].entry & PAGE_PRESENT)) {
        return; // Error: PDPT doesn't exist
    }

    if (!is_present(&pml4[pml4_index])) return;
    page_table_entry_t *pdpt = (page_table_entry_t*)get_physical_address(&pml4[pml4_index]);


    // Check if PDPT entry exists
    if (!(pdpt[pdpt_index].entry & PAGE_PRESENT)) {
        return; // Error: PD doesn't exist
    }

    if (!is_present(&pdpt[pdpt_index])) return;
    page_table_entry_t *pd = (page_table_entry_t*)get_physical_address(&pdpt[pdpt_index]);

    // Check if this is a 2MB page
    if (pd[pd_index].entry & PAGE_LARGE) {
        // Unmap the 2MB page
        pd[pd_index].entry = 0;
        return; // Success
        invlpg((void*)virt);
    }

    // Check if PD entry exists for 4K page
    if (!(pd[pd_index].entry & PAGE_PRESENT)) {
        return; // Error: PT doesn't exist
    }

    page_table_entry_t *pt = (page_table_entry_t*)get_physical_address(&pd[pd_index]);
    
    // Unmap the 4K page
    pt[pt_index].entry = 0;
    invlpg((void*)virt);

    return; // Success
}


// Page fault handler

void page_fault_handler(uint64_t error_code) {
    log(LOG_LEVEL_DEBUG, "Page Fault\n");
    log(LOG_LEVEL_DEBUG, "Error Code %ld", error_code);
    // Handle the exception or halt the CPU
    while(1){
        asm("hlt");
    }
}

/* Process paging functions
*
*/

typedef struct {
    uint64_t entry;
} PageTableEntry;

typedef struct {
    PageTableEntry entries[ENTRIES_PER_TABLE];
} PageTable;

/*
* Clones the current paging structure
* Clears the lower half of the virtual address space
* 
*/
PageTable* clone_higher_half(uint64_t *original_pml4) {
    PageTable *new_pml4 = (PageTable*)aligned_alloc(PAGE_SIZE, PAGE_SIZE);
    if (!new_pml4) return NULL;

    // Initialize the new PML4 to 0
    memset(new_pml4, 0, PAGE_SIZE);

   // Copy the PML4
    memcpy(new_pml4, original_pml4, PAGE_SIZE);

    // Iterate through PML4 entries
    for (int pml4_index = 0; pml4_index < ENTRIES_PER_TABLE; pml4_index++) {
        if (new_pml4->entries[pml4_index].entry & 1) {  // If present
            PageTable *pdpt = (PageTable*)(new_pml4->entries[pml4_index].entry & ~0xFFF);
            PageTable *new_pdpt = (PageTable*)aligned_alloc(PAGE_SIZE, PAGE_SIZE);
            if (!new_pdpt) {
                // Handle error (free allocated memory)
                return NULL;
            }
            memcpy(new_pdpt, pdpt, PAGE_SIZE);
            new_pml4->entries[pml4_index].entry = (uint64_t)new_pdpt | (new_pml4->entries[pml4_index].entry & 0xFFF);

            // Iterate through PDPT entries
            for (int pdpt_index = 0; pdpt_index < ENTRIES_PER_TABLE; pdpt_index++) {
                if (new_pdpt->entries[pdpt_index].entry & 1) {  // If present
                    PageTable *pd = (PageTable*)(new_pdpt->entries[pdpt_index].entry & ~0xFFF);
                    PageTable *new_pd = (PageTable*)aligned_alloc(PAGE_SIZE, PAGE_SIZE);
                    if (!new_pd) {
                        // Handle error (free allocated memory)
                        return NULL;
                    }
                    memcpy(new_pd, pd, PAGE_SIZE);
                    new_pdpt->entries[pdpt_index].entry = (uint64_t)new_pd | (new_pdpt->entries[pdpt_index].entry & 0xFFF);

                    // Iterate through PD entries
                    for (int pd_index = 0; pd_index < ENTRIES_PER_TABLE; pd_index++) {
                        if (new_pd->entries[pd_index].entry & 1) {  // If present
                            // Check if it's a 2MB page (bit 7 set)
                            if (!(new_pd->entries[pd_index].entry & (1 << 7))) {
                                PageTable *pt = (PageTable*)(new_pd->entries[pd_index].entry & ~0xFFF);
                                PageTable *new_pt = (PageTable*)aligned_alloc(PAGE_SIZE, PAGE_SIZE);
                                if (!new_pt) {
                                    // Handle error (free allocated memory)
                                    return NULL;
                                }
                                memcpy(new_pt, pt, PAGE_SIZE);
                                new_pd->entries[pd_index].entry = (uint64_t)new_pt | (new_pd->entries[pd_index].entry & 0xFFF);
                            }
                        }
                    }
                }
            }
        }
    }

    return new_pml4;
}

void new_paging(){

    uint64_t cr3 = read_cr3();
    //uint64_t *original_pml4 = (uint64_t *)(cr3 & ~0xFFFULL); /* Your original PML4 address */;
    if(bootboot_paging == NULL)   bootboot_paging = (cr3 & ~0xFFFULL);

    PageTable *new_pml4 = clone_higher_half(bootboot_paging);

    if (new_pml4) {
        // You now have a clone of the higher half paging structure
        // You may need to update certain mappings or merge this with your existing lower half
        //paging_switch((uint64_t)(void*)new_pml4);
        write_cr3((uint64_t)(void*)new_pml4);
        
        //map_4kb_page();
        log(LOG_LEVEL_INFO, "Page switch successful!!!!");
               
        
        //terminal_write("Hello World\n");
    } else {
        // Handle error of no memory, in this case the process creation fails as there is no memory for setting up a program page table
    }
}
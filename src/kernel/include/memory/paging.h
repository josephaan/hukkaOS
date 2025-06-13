/*
 * paging.h
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
 * This file is part of the HukkaOS. 
 * 
 *
 */

#ifndef __PAGING__H
#define __PAGING__H

#include <stddef.h>
#include <stdint.h>



#define PAGE_PRESENT      0x1
#define PAGE_WRITABLE   0x2
#define PAGE_READ_WRITE   0x3
#define PAGE_USER         0x4

#define PTE_PRESENT (1ULL << 0)
#define PTE_WRITABLE (1ULL << 1)
#define PTE_USER (1ULL << 2)
#define PTE_WRITE_THROUGH (1ULL << 3)
#define PTE_CACHE_DISABLE (1ULL << 4)
#define PTE_ACCESSED (1ULL << 5)
#define PTE_DIRTY (1ULL << 6)
#define PTE_HUGE (1ULL << 7)
#define PTE_GLOBAL (1ULL << 8)
#define PTE_NX (1ULL << 63)

#define VMM_FLAG_PRESENT        (1 << 0)    /* P   */
#define VMM_FLAG_READWRITE      (1 << 1)    /* R/W */
#define VMM_FLAG_USER           (1 << 2)    /* U/S */
#define VMM_FLAG_WRITETHROUGH   (1 << 3)    /* PWT */
#define VMM_FLAG_CACHE_DISABLE  (1 << 4)    /* PCD */
#define VMM_FLAG_PAT            (1 << 7)    /* PAT */

#define PTE_DEFAULT_FLAGS       (PTE_PRESENT | PTE_WRITABLE)

/* According to Intel's manual, only when CACHE_DISABLE and WRITETHROUGH
 * are both set to 1, the MMIO will be strong uncacheable (UC) which can
 * meet requirements of some memory regions, e.g., xAPIC memory address.
 * If we only set CACHE_DISABLE value, thw writing operation will be halt
 * when running on NEC VersaPro which has a i5-6200U CPU.
 */
#define PTE_MMIO_FLAGS          (PTE_DEFAULT_FLAGS | PTE_CACHE_DISABLE | PTE_WRITE_THROUGH)


#define PAGE_SIZE    4096         // 4kb PAGE size
#define PAGE_LARGE   2*1024*1024  // 2mb PAGE size
#define PAGE_ENTRIES 512


// Page Table entry structure
typedef struct {
    uint64_t entries[PAGE_ENTRIES];
} page_table_t;

typedef struct {
    uint64_t entry;
} page_table_entry_t;
//
void traverse_paging_structure(void) ;
uint64_t translate_virtual_to_physical(uint64_t virtual_address);

void init_paging();

void kernel_virtual_start(void);
void kernel_virtual_end(void);

void new_paging();

// mapping function

void map_4kb_page(uint64_t virtual_addr, uint64_t physical_addr, uint64_t flags);
void map_2mb_page(uint64_t virtual_addr, uint64_t physical_addr, uint64_t flags);
void unmap_page(uint64_t virt);

void page_fault_handler(uint64_t error_code);

#endif
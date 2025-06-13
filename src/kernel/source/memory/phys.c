#include <memory/phys.h>
#include <x86_64/cpu.h>
#include <debug/log.h>
#include <bootboot.h>
#include <display/framebuffer.h>

extern BOOTBOOT bootboot;

#define PAGE_SIZE 4096
#define BITS_PER_WORD 64
#define MAX_FREE_BLOCKS 32  // Adjust this based on your expected maximum number of free blocks

typedef struct {
    uint64_t base_address;
    uint64_t size;
    uint8_t type;
} MemoryBlock;

typedef struct {
    uint64_t *bitmap;
    uint64_t base_address;
    uint64_t size;
    uint64_t num_pages;
} BitmapManager;

static uint64_t free_block_count = 0;

static BitmapManager phys_mblocks[MAX_FREE_BLOCKS]; // physical memory blocks


void setup_physical_memory_management() {


    MMapEnt* mmap_ent = &bootboot.mmap;
  
    uint64_t num_mmap_entries = (bootboot.size - 128) / sizeof(MMapEnt);
    uint64_t base_addr = 0 ;
    uint64_t size = 0 ;
    uint32_t type = 0 ;
    
    for (uint64_t i = 0; i < num_mmap_entries; i++) {
        base_addr = MMapEnt_Ptr(mmap_ent);
        size = MMapEnt_Size(mmap_ent);
        type = MMapEnt_Type(mmap_ent);

        if(type == MMAP_FREE){

            // Allocate the bitmap at the beginning of the free block
            phys_mblocks[free_block_count].bitmap = (uint64_t *)base_addr;
            
            // Calculate the number of pages in the block
            phys_mblocks[free_block_count].num_pages = size / PAGE_SIZE;
            
            // Calculate the size of the bitmap in words (uint64_t)
            uint64_t total_pages = phys_mblocks[free_block_count].num_pages;
            uint64_t total_bitmaps = (total_pages + 63) / 64;
            
            phys_mblocks[free_block_count].base_address = base_addr + (total_bitmaps * sizeof(uint64_t));

            
            // Initialize the bitmap (set all bits to 1, indicating free pages)
            for (uint64_t j = 0; j < total_bitmaps; j++) {
                phys_mblocks[free_block_count].bitmap[j] = 0xFFFFFFFFFFFFFFFF;
            }
            
            // Mark the pages used by the bitmap as occupied
            uint64_t bitmap_pages = (total_bitmaps * sizeof(uint64_t) + PAGE_SIZE - 1) / PAGE_SIZE;
            for (uint64_t k = 0; k < bitmap_pages; k++) {
                phys_mblocks[free_block_count].bitmap[k / BITS_PER_WORD] &= ~(1ULL << (k % BITS_PER_WORD));
            }
            free_block_count++;
        }
        mmap_ent++;
    }

    log(LOG_LEVEL_DEBUG, "Physical Memory Initialized");
    
}

void print_physical_memory(){
    for(uint32_t i = 0; i < free_block_count; i++){
        print_numstr("\nEntry [%d]: Address [%p]", 0xd0d1d2, i, phys_mblocks[i].base_address);
        enum color Color = Yellow;
        if (i == 0 || i == 1) 
            for(uint32_t j = 0;  j < ((phys_mblocks[i].num_pages + 63)/64) ; j++) print_numstr("\nBitmap [%p]", Color, phys_mblocks[i].bitmap[j]);
            //for(uint32_t j = 0;  j < 1 ; j++) print_numstr("\nBitmap [%p]", Color, phys_mblocks[i].bitmap[j]);
    }
}

// Function to find first free page
static int find_first_free_page(BitmapManager phys_mblocks[]) {
    for (size_t i = 0; i < phys_mblocks->num_pages; i++) {
        if (phys_mblocks->bitmap[i / BITS_PER_WORD] & (1ULL << (i % BITS_PER_WORD))) {
            return i;
        }
    }
    return -1; // No free page found
}

// Function to allocate a single page
void *bitmap_allocate_page() {
    for (size_t i = 0; i < free_block_count; i++) {
        int free_page_index = find_first_free_page(&phys_mblocks[i]);
        if (free_page_index != -1) {
            // Mark the page as used
            phys_mblocks[i].bitmap[free_page_index / BITS_PER_WORD] &= ~(1ULL << (free_page_index % BITS_PER_WORD));
            // Calculate and return the address of the allocated page
            return (void *)(phys_mblocks[i].base_address + (uint64_t)free_page_index * PAGE_SIZE);
        }
            
    }
    return NULL; // No free page available
    log(LOG_LEVEL_ERROR, "OUT OF MEMORY");
}

// Function to free a single page
bool bitmap_free_page(void *page_address) {
    for (size_t i = 0; i < free_block_count; i++) {
        if (page_address >= (void *)phys_mblocks[i].base_address && 
            page_address < (void *)(phys_mblocks[i].base_address + phys_mblocks[i].num_pages * PAGE_SIZE)) {
            
            uint64_t page_index = ((uint64_t)page_address - phys_mblocks[i].base_address) / PAGE_SIZE;
            
            // Mark the page as free
            phys_mblocks[i].bitmap[page_index / BITS_PER_WORD] |= (1ULL << (page_index % BITS_PER_WORD));
            
            return true; // Page successfully freed
        }
    }
    return false; // Page address not found in any managed block
}


// Multiple pages allocation

// Function to find first sequence of free pages
static int find_contiguous_free_pages(BitmapManager phys_mblocks[], uint64_t num_pages) {
    uint64_t contiguous_count = 0;
    for (size_t i = 0; i < phys_mblocks->num_pages; i++) {
        if (phys_mblocks->bitmap[i / BITS_PER_WORD] & (1ULL << (i % BITS_PER_WORD))) {
            contiguous_count++;
            if (contiguous_count == num_pages) {
                return i - num_pages + 1;  // Return the start index
            }
        } else {
            contiguous_count = 0;
        }
    }
    return -1; // No contiguous free pages found
}


// Function to allocate multiple contiguous pages
void *bitmap_allocate_pages(uint64_t num_pages) {
    for (size_t i = 0; i < free_block_count; i++) {
        int start_page = find_contiguous_free_pages(&phys_mblocks[i], num_pages);
        if (start_page != -1) {
            // Mark the pages as used
            for (uint64_t j = 0; j < num_pages; j++) {
                uint64_t page_index = start_page + j;
                phys_mblocks[i].bitmap[page_index / BITS_PER_WORD] &= ~(1ULL << (page_index % BITS_PER_WORD));
            }
            
            // Calculate and return the address of the first allocated page
            return (void *)(phys_mblocks[i].base_address + (uint64_t)start_page * PAGE_SIZE);
        }
    }
    return NULL; // No contiguous free pages available
}

// Function to free multiple pages
bool bitmap_free_pages(void *page_address, uint64_t num_pages) {
    for (size_t i = 0; i < free_block_count; i++) {
        if (page_address >= (void *)phys_mblocks[i].base_address && 
            page_address < (void *)(phys_mblocks[i].base_address + phys_mblocks[i].num_pages * PAGE_SIZE)) {
            
            uint64_t start_page_index = ((uint64_t)page_address - phys_mblocks[i].base_address) / PAGE_SIZE;
            
            // Check if all pages are within this manager's range
            if (start_page_index + num_pages <= phys_mblocks[i].num_pages) {
                // Mark the pages as free
                for (uint64_t j = 0; j < num_pages; j++) {
                    uint64_t page_index = start_page_index + j;
                    phys_mblocks[i].bitmap[page_index / BITS_PER_WORD] |= (1ULL << (page_index % BITS_PER_WORD));
                }
                
                return true; // Pages successfully freed
            }
        }
    }
    return false; // Page address not found or not enough contiguous pages in any managed block
}

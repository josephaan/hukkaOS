#include <memory/pmm.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <bootboot.h>
#include <display/framebuffer.h>
#include <debug/log.h>
#include <bootboot.h>

#include <hardware/lock/spinlock.h>

extern BOOTBOOT bootboot;

#define PAGE_SIZE 4096  // 4KB pages

void parse_memory_map() {
    MMapEnt* mmap_ent = &bootboot.mmap;
  
    uint64_t num_mmap_entries = (bootboot.size - 128) / sizeof(MMapEnt),  total_memory = 0, free_memory = 0;

    print_numstr("\nNumber of memory entry = %lu entries \n ", 0x00AA00, num_mmap_entries);
    
    for (uint64_t i = 0; i < num_mmap_entries; i++) {
        uint64_t base_addr = MMapEnt_Ptr(mmap_ent);
        uint64_t size = MMapEnt_Size(mmap_ent);
        uint32_t type = MMapEnt_Type(mmap_ent);
        
        total_memory += size;
        if(base_addr == 0UL) {
                base_addr = base_addr + 1;
                size = size - 1;
        }
        print_numstr("Entry %lu: Base Address: %p, Size: %lu kb |  %lu Mb, Type: %d\n", 0x00AA00,i, (void*)base_addr, size/(1024), size/(1024 * 1024), type);
        if(type == MMAP_FREE){
            free_memory += size;
        }
        
        mmap_ent++;
    }
    print_numstr("The Total memory = %lu kb or %lu mb or %lu Gb", 156,  total_memory/1024, total_memory/(1024*1024), total_memory/(1024*1024*1024));
    print_numstr("\nThe Free memory = %lu kb or %lu mb or %lu Gb", 156,  free_memory/1024, free_memory/(1024*1024), free_memory/(1024*1024*1024));
}

void calculate_memory_usage(uint64_t *total_memory, uint64_t *free_memory, uint64_t *used_memory) {
    MMapEnt* mmap_ent = &bootboot.mmap;
    uint64_t num_mmap_entries = (bootboot.size - 128) / sizeof(MMapEnt);

    *total_memory = 0;
    *free_memory = 0;
    *used_memory = 0;

    for (uint64_t i = 0; i < num_mmap_entries; i++) {
        uint64_t size = MMapEnt_Size(mmap_ent);
        uint8_t type = MMapEnt_Type(mmap_ent);

        *total_memory += size;

        if (type == MMAP_FREE) {
            *free_memory += size;
        } else {
            *used_memory += size;
        }

        mmap_ent++;
    }
}

// Due to the usage of NULL, its recommended that the address (void*)0 be avoided if available for allocation
// Matter of fact during testing it does not appear in the list of free mmap for bios but does for uefi

#define PAGE_SIZE_4K 4096         // 4Kb pages
#define PAGE_SIZE_2M 2*1024*1024  // 2Mb Pages
#define NUM_BLOCKS   12000           // Total number of blocks

static uint16_t free_blocks   = 0;  // This variable should be used wisely as it keep track of the free memory entries
static uint16_t used_blocks   = 0;
static uint64_t start_address = 0;
static uint64_t total_memory = 0;


Memory_Block free_memory_blocks[NUM_BLOCKS], used_memory_blocks[NUM_BLOCKS];


atomic_flag init_memory_lock = ATOMIC_FLAG_INIT;

void init_blocks(){

    acquire_lock(&init_memory_lock);

    MMapEnt* mmap_ent = &bootboot.mmap;
  
    uint64_t num_mmap_entries = (bootboot.size - 128) / sizeof(MMapEnt);
    uint64_t base_addr ;
    uint64_t size ;
    uint32_t type ;
    uintptr_t current_free_block = 0;
    
    for (uint64_t i = 0; i < num_mmap_entries; i++) {
        base_addr = MMapEnt_Ptr(mmap_ent);
        size = MMapEnt_Size(mmap_ent);
        type = MMapEnt_Type(mmap_ent);
        total_memory += size;


        if(type == MMAP_FREE){

            if(base_addr == 0UL) {
                base_addr = base_addr + 1;
                size = size - 1;
            }

            free_memory_blocks[free_blocks].address = base_addr;
            free_memory_blocks[free_blocks].size    = size;
            free_memory_blocks[free_blocks].next_block = (Memory_Block*)current_free_block;
            current_free_block = base_addr;
            free_blocks++;

        } else {
            used_memory_blocks[used_blocks].address = base_addr;
            used_memory_blocks[used_blocks].size    = size;
            used_blocks++;
        }

        if(i == 0) start_address = base_addr ;
        mmap_ent++;
    }

    release_lock(&init_memory_lock);

}

void physical_memory_init(){
    init_blocks();
}

void print_blocks(){
    init_blocks();
    print_numstr("\n\nTotal Memory = %ld kb\n", 0x045fe, total_memory/1024);
    for (uint8_t i = 0; i < free_blocks; i++){
        print_numstr("Adrress: %p , size: %ld kb\n", 0x7ff4, free_memory_blocks[i].address, free_memory_blocks[i].size/(1024));
    }
}


uint64_t find_block(uint64_t address, Memory_Block block[]) {
    for (size_t i = 0; i < NUM_BLOCKS; i++) {
        if (block[i].address == address) {
            return block[i].size;
        }
    }
    return (uint64_t)NULL;
}

void remove_block(uint64_t address, Memory_Block block[]) {
    for (size_t i = 0; i < NUM_BLOCKS; i++) {
        if (block[i].address == address) {
            block[i].address = (uint64_t)NULL;
            block[i].size    = (uint64_t)NULL;
        }
    }
}


void *__attribute__((warn_unused_result)) allocate_memory(uint64_t size)  {
    for (size_t i = 0; i < free_blocks; i++) {
        if(free_memory_blocks[i].size >= size) {
            uintptr_t address = free_memory_blocks[i].address;
            free_memory_blocks[i].address = free_memory_blocks[i].address + size ;
            free_memory_blocks[i].size = free_memory_blocks[i].size - size;
            return (void*)address;
        }
    }
    return NULL;
}

void free_memory(uint64_t address){

    // Remove_block from used memory list using the address
    // Checks the free list for a neighbor | address-1 or address + size
    // if such a neighbour exist| on left than add size to neighbour size
    // if on the right than add size to neighbour and change neighbour address to the address

    uint64_t size = find_block(address, used_memory_blocks);
    if(size != (uint64_t)NULL) {
        uint64_t front_neighbor = 0;
        uint64_t back_neighbor = 0;
        uint8_t f_neighbor_pos = (uint64_t)NULL;
        uint8_t b_neighbor_pos = (uint64_t)NULL;
        for (uint8_t i = 0; i < free_blocks; i++) {
            uint64_t n_address = free_memory_blocks[i].address;
            uint64_t n_size    = free_memory_blocks[i].size;
            if( (n_address + n_size + 1) == address   ) { front_neighbor = n_address ; f_neighbor_pos = i; }
            if( (address   + size   + 1) == n_address ) { back_neighbor  = n_address ; b_neighbor_pos = i; }
            
        }
        if(front_neighbor != (uint64_t)NULL){
            free_memory_blocks[f_neighbor_pos].size += size;
            remove_block(address, used_memory_blocks); 
        } else if (back_neighbor != (uint64_t)NULL){
            free_memory_blocks[b_neighbor_pos].address = address;
            remove_block(address, used_memory_blocks); 
        } else {
            free_memory_blocks[free_blocks+1].address = address;
            free_memory_blocks[free_blocks+1].size = size;
            free_blocks++;
            remove_block(address, used_memory_blocks); 
        }

    }
}

void *__attribute__((warn_unused_result)) aligned_alloc(size_t alignment, size_t size) {
    if (alignment < sizeof(void*)) {
        alignment = sizeof(void*);
    }
    
    if ((alignment & (alignment - 1)) != 0) {
        // Alignment is not a power of two
        return NULL;
    }

    // Allocate extra memory to ensure we can get an aligned block
    void* original = allocate_memory(size + alignment - 1 + sizeof(void*));
    if (original == NULL) {
        return NULL;
    }

    // Align the pointer
    void* aligned = (void*)(((uintptr_t)original + alignment - 1 + sizeof(void*)) & ~(alignment - 1));

    // Store the original pointer just before the aligned memory block
    ((void**)aligned)[-1] = original;

    return aligned;
}

void aligned_free(void* ptr) {
    if (ptr) {
        free_memory((uint64_t)((void**)ptr)[-1]);
    }
}


void* allocate_page(){
    return allocate_memory(PAGE_SIZE);
}
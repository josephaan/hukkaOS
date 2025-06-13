#include <memory/pmm.h>
#include <memory/paging.h>
#include <memory/vmm.h>

// This function is to be used when creating a new user process, 
void init_user_program_memory(){
    uintptr_t init_physical_mem = (uintptr_t)allocate_memory(PAGE_SIZE*4);
    uintptr_t stack_memory_physical = (uintptr_t)allocate_memory(PAGE_SIZE);//  aligned_alloc(4096,4096);
    uintptr_t stack_virt_addr = USER_SPACE_END - 0x1000;
    if (init_physical_mem != NULL){
        if(stack_memory_physical != NULL){
            for (int i = 0; i < 4; i++){
                map_4kb_page(USER_PROGRAM_START + (i * 0x1000), init_physical_mem+(i * 0x1000), PTE_DEFAULT_FLAGS | PTE_USER );
                unmap_page(init_physical_mem+(i * 0x1000));
            }
            map_4kb_page(stack_virt_addr, stack_memory_physical, PTE_DEFAULT_FLAGS | PTE_USER);
        }
        else{
            // Handle error of no memory available for stack
        }
    }
    else{
        // Handle error of no memory available to start program
    }
    
}


// allocates kernel memory
void* kmalloc(size_t size){
    return NULL;
}
void kfree(void* ptr){
    
}
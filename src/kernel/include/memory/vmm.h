#ifndef __VMM__H
#define __VMM__H

#include <stdint.h>
#include <stddef.h>

#define USER_PROGRAM_START 0x0000000400000000 
#define USER_SPACE_END     0x00007FFFFFFFFFFF
#define KERNEL_SPACE_START 0xFFFF800000000000
#define USER_STACK_START   0x00007FFFFFFFEFFF

void init_user_program_memory();
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif // !__VMM__H


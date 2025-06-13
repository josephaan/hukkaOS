#ifndef __PHYS__H
#define __PHYS__H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void setup_physical_memory_management();
void print_physical_memory();

bool bitmap_free_page(void *page_address);
void *bitmap_allocate_page();

#endif // !__PHYS__H

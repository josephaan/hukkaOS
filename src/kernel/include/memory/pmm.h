#ifndef __PMM__H
#define __PMM__H

#pragma once //;

#include <bitwise.h>


typedef struct memory_block{
    uint64_t address;
    uint64_t size;
    struct memory_block * next_block;
} Memory_Block;

/* List of various memories */

void print_blocks();
uint64_t find_block(uint64_t address, Memory_Block block[]);

uint64_t calculate_total_memory();

void parse_memory_map();

void calculate_memory_usage(uint64_t* total_memory, uint64_t* free_memory, uint64_t* used_memory);

void physical_memory_init();

void *allocate_memory(uint64_t size);

void* allocate_page();

void *__attribute__((warn_unused_result)) aligned_alloc(size_t alignment, size_t size);

void free_memory(uint64_t address);


#endif // !__PMM__H

#ifndef __PROCESS__H
#define __PROCESS__H

#include <stdint.h>
#include <x86_64/cpu.h>


// Process State

#define NEW_PROCESS        0
#define RUNNING_PROCESS    1
#define READY_PROCESS      2
#define WAITING_PROCESS    3
#define SUSPENDED_PROCESS  4
#define BLOCKED_PROCESS    5
#define KILLED_PROCESS     6


#define STACK_SIZE 1024;
#define MAX_PROCESS_NAME 32
#define MAX_PROCESSES 100
#define MAX_OPEN_FILES 16
#define MAX_THREADS 16

typedef struct threads{
    uint64_t thread_id;
    char thread_name[MAX_PROCESS_NAME];

    enum {
        PROCESS_NEW,
        PROCESS_RUNNING,
        PROCESS_READY,
        PROCESS_WAITING,
        PROCESS_SUSPENDED,
        PROCESS_BLOCKED,
        PROCESS_KILLED
    } state;   

    // CPU context
    cpu_status_t context;

    // Memory Management
    uint64_t thread_stack[1024];     // proc stack area

    // Scheduling information
    uint64_t time_slice;              // Allocated CPU time slice
    uint64_t priority;                // Scheduling priority
    uint64_t runtime;                 // Total runtime

    // Parent process
    struct ProcessControlBlock* parent;

    // Pointers for process list management
    struct threads* next;

} thread_t;

typedef struct process_control_block {
    uint64_t process_id;               // Process ID
    char name[MAX_PROCESS_NAME];      // Process name
    
    // Memory management
    void* page_table;                 // Pointer to page table
    uint64_t heap_start;              // Start of heap
    uint64_t heap_size;               // Current heap size
    

    // File management
    int open_files[MAX_OPEN_FILES];   // Array of file descriptors

    // Threads
    thread_t * process_threads;
    uint32_t thread_count;

    // Parent process
    struct ProcessControlBlock* parent;

    // Pointers for process list management
    struct ProcessControlBlock* next;
    struct ProcessControlBlock* prev;
} process_control_block_t;

#define TIME_SLICE    10 // milliseconds

#endif // __PROCESS__H

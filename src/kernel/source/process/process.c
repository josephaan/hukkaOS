#include <process/process.h>
#include <x86_64/cpu.h>
#include <string.h>
#include <stddef.h>
#include <memory/pmm.h>
#include <memory/paging.h>
#include <memory/vmm.h>
#include <debug/log.h>



size_t assigned_process_id = 0; // The current number is the last id assigned to a process;
size_t next_thread_id = 0; // The current number is the last id assigned to a thread;

void init_process(){
    
}

process_control_block_t * create_kernel_process(char* name) {
    // Add a function to disable interrupt so as for a process creation is fully done before an interrupt can occur
    assigned_process_id++;

    process_control_block_t * process = allocate_memory(sizeof(process_control_block_t));
    strncpy(process->name, name, MAX_PROCESS_NAME);
    process->process_id = assigned_process_id;

    return process;
}

process_control_block_t * create_user_process(char* name) {
    // Add a function to disable interrupt so as for a process creation is fully done before an interrupt can occur
    assigned_process_id++;

    process_control_block_t * process = allocate_memory(sizeof(process_control_block_t));
    strncpy(process->name, name, MAX_PROCESS_NAME);
    process->process_id = assigned_process_id;
    new_paging();
    init_user_program_memory();

    //process->page_table = create_process_page_table();

    return process;
}


thread_t * create_kernel_thread(process_control_block_t* process, char* name, void(*function)(void*), void* arg){
    thread_t* thread = allocate_memory(sizeof(thread_t));
    if (process->process_threads == NULL)
        process->process_threads[0] = *thread;
    else {
        for (thread_t* scan = process->process_threads; scan != NULL; scan = scan->next) {
            if (scan->next != NULL)
                continue;
            scan->next = thread;
            break; 
        } 
    }
    strncpy(thread->thread_name, name, MAX_PROCESS_NAME);
    thread->thread_id = next_thread_id++;
    thread->state = NEW_PROCESS;
    thread->next = NULL;
    thread->context.ss = KERNEL_SS;
    thread->context.rsp = allocate_memory(1024*1024);
    thread->context.rflags = 0x202;
    thread->context.cs = KERNEL_CS;
    thread->context.rip = (uint64_t)function;
    thread->context.rdi = (uint64_t)arg;
    thread->context.rbp = 0;
    return thread;

}


thread_t * create_user_thread(process_control_block_t* process, char* name, void (*main)(), void* arg){
    disable_interrupts();
    thread_t* thread = allocate_memory(sizeof(thread_t));
    if (process->process_threads == NULL)
        process->process_threads[0] = *thread;
    else {
        for (thread_t* scan = process->process_threads; scan != NULL; scan = scan->next) {
            if (scan->next != NULL)
                continue;
            scan->next = thread;
            break; 
        } 
    }
    strncpy(thread->thread_name, name, MAX_PROCESS_NAME);
    thread->thread_id = next_thread_id++;
    thread->state = NEW_PROCESS;
    thread->next = NULL;
    thread->context.ss = USER_SS;
    thread->context.rsp = (uint64_t)USER_SPACE_END; // if a faut occurs here it might be due to the fact that qemu , USER_SPACE_END - 0x1000 = 0x00007FFFFFFFE000 but the end adress is 0x00007FFFFFFFF000 not 0x00007FFFFFFFFFFF 
    thread->context.rflags = 0x202;
    thread->context.cs = USER_CS;
    thread->context.rip = (uint64_t)main;
    thread->context.rdi = (uint64_t)arg;
    thread->context.rbp = 0;

    log(LOG_LEVEL_INFO, "Thread %s successfully created with pid %lu: ", name, thread->thread_id);
    return thread;

    enable_interrupts();

}
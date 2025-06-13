#include <process/process.h>
#include <hardware/lock/spinlock.h>
#include <stddef.h>
#include <memory/pmm.h>

int current_process = 0;
int num_processes = 0;

process_control_block_t * process_table;

// Initialize the scheduler
void init_scheduler() {
    process_table = allocate_memory(sizeof(process_control_block_t)*MAX_PROCESSES);
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].process_id = 0; // Mark as unused, 0 pid is an unitialized process, it is never run
    }
}

// Add a process to the scheduler
int32_t add_process(uint32_t *stack_pointer, uint32_t *page_directory) {
    if (num_processes >= MAX_PROCESSES) return -1; // No space

}

static thread_t * current_thread = NULL;


void process_scheduler(){

    if (!current_thread) {
        // First scheduling, find the first ready thread
        for (process_control_block_t* p = process_table; p != NULL; p = (process_control_block_t*)p->next) {
            for (uint32_t i = 0; i < p->thread_count; i++) {
                if (p->process_threads[i].state == READY_PROCESS) {
                    current_thread = &p->process_threads[i];
                    current_thread->state = RUNNING_PROCESS;
                    return;
                }
            }
        }
    } else {
        // Simple round-robin scheduling
        thread_t* next_thread = NULL;
        process_control_block_t* current_process = (process_control_block_t*)current_thread->parent;

        // First, try to find the next thread in the same process
        for (int i = 0; i < current_process->thread_count; i++) {
            if (&current_process->process_threads[i] == current_thread) {
                for (int j = (i + 1) % current_process->thread_count; j != i; j = (j + 1) % current_process->thread_count) {
                    if (current_process->process_threads[j].state == READY_PROCESS) {
                        next_thread = &current_process->process_threads[j];
                        break;
                    }
                }
                break;
            }
        }

        // If no thread found in the same process, look in other processes
        if (!next_thread) {
            process_control_block_t* p = current_process->next ? current_process->next : process_table;
            while (p != current_process) {
                for (uint32_t i = 0; i < p->thread_count; i++) {
                    if (p->process_threads[i].state == READY_PROCESS) {
                        next_thread = &p->process_threads[i];
                        break;
                    }
                }
                if (next_thread) break;
                p = p->next ? p->next : process_table;
            }
        }

        if (next_thread) {
            current_thread->state = READY_PROCESS;
            next_thread->state = RUNNING_PROCESS;
            current_thread = next_thread;
        }
    }

}

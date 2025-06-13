#ifndef __SYSCALL__H
#define __SYSCALL__H

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <x86_64/cpu.h>

/*

Ah, I see. Building a small kernel is an excellent learning project. For a basic kernel, there are several fundamental system calls you might consider implementing. Here are some suggestions:

    write: For outputting data (e.g., to the console)
    read: For reading input
    exit: To terminate a process
    fork: To create a new process (if you're implementing multi-tasking)
    exec: To execute a program
    open: To open files
    close: To close file descriptors
    wait: For process synchronization

These are some of the most basic and commonly used system calls. They provide core functionality for process management, I/O operations, and file handling.

If you're just starting, you might want to begin with write and exit. These will allow you to output information and properly terminate processes, which are crucial for debugging and basic functionality.

As you progress, you can add more complex system calls like fork and exec to support multi-tasking and program execution.

Would you like me to explain any of these system calls in more detail, or discuss how you might go about implementing one of them?

*/

// SYSCALLS

#define SYSCALL_RESERVED 0x00
#define SYSCALL_WRITE    0x01 // For outputting data (e.g., to the console)
#define SYSCALL_READ     0x02 // For reading input
#define SYSCALL_EXIT     0x03 // To terminate a process
#define SYSCALL_FORK     0x04 // To create a new process (if you're implementing multi-tasking)
#define SYSCALL_EXECUTE  0x05 // To execute a program
#define SYSCALL_OPEN     0x06 // To open files
#define SYSCALL_CLOSE    0x07 // To close file descriptors
#define SYSCALL_WAIT     0x08 // For process synchronization



cpu_status_t* system_call_handler(cpu_status_t *registers);

#endif // !__SYSCALL__H

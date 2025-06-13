#include <hukaos.h>




uint64_t syscall(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    uint64_t ret;
    asm volatile (
        "movq %1, %%rax\n"
        "movq %2, %%rdi\n"
        "movq %3, %%rsi\n"
        "movq %4, %%rdx\n"
        "syscall\n"
        "movq %%rax, %0"
        : "=r" (ret)
        : "r" (syscall_number), "r" (arg1), "r" (arg2), "r" (arg3)
        : "rax", "rdi", "rsi", "rdx", "rcx", "r11"
    );
    return ret;
}
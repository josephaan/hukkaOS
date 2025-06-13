#include <hardware/io/io.h>

// Byte (8-bit) port I/O functions
void out_b(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t in_b(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// 16-bit port I/O functions
void out_w(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

uint16_t in_w(uint16_t port) {
    uint16_t value;
    __asm__ volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// 32-bit port I/O functions
void out_l(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

uint32_t in_l(uint16_t port) {
    uint32_t value;
    __asm__ volatile ("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// 64-bit port I/O functions (custom implementation)
void out_q(uint16_t port, uint64_t value) {
    out_l(port, (uint32_t)(value & 0xFFFFFFFF));
    out_l(port + 4, (uint32_t)(value >> 32));
}

uint64_t in_q(uint16_t port) {
    uint64_t value = in_l(port);
    value |= ((uint64_t)in_l(port + 4) << 32);
    return value;
}

void in_sw(uint16_t port, void* addr, unsigned int count){
    __asm__ volatile ("rep insw"
        : "+D"(addr), "+c"(count)
        : "d"(port)
        : "memory"
    );

}
void out_sw(uint16_t port, void* addr, unsigned int count){
    __asm__ volatile(
        "rep outsw"
        : "+S"(addr), "+c"(count)
        : "d"(port)
        : "memory"
    );
}
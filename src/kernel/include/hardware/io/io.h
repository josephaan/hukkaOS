#ifndef __IO__H
#define __IO__H

#include <stdint.h>

// Byte (8-bit) port I/O functions
void out_b(uint16_t port, uint8_t value);
uint8_t in_b(uint16_t port) ;

// 16-bit port I/O functions
void out_w(uint16_t port, uint16_t value);
uint16_t in_w(uint16_t port);

// 32-bit port I/O functions
void out_l(uint16_t port, uint32_t value);
uint32_t in_l(uint16_t port) ;

// 64-bit port I/O functions (custom implementation)
void out_q(uint16_t port, uint64_t value) ;
uint64_t in_q(uint16_t port);

void in_sw(uint16_t port, void* addr, unsigned int count);
void out_sw(uint16_t port, void* addr, unsigned int count);

#endif // !__IO__H

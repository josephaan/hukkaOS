#include <bitwise.h>

uint64_t flip_bit(uint64_t num, uint64_t n) {

	// Create a mask with a 1 at position n
	uint64_t mask = 1 << n;
	// Use XOR to flip the bit to position n
	num ^= mask;
	return num;
}

uint8_t check_bit(uint32_t value, int bit_position) {
    return (value & (1 << bit_position)) != 0;
}
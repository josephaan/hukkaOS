#include <hardware/ps2/ps2.h>

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(data), "Nd"(port));
}

static uint8_t inb(uint16_t port) {
    unsigned char data = 0;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

// Function to send a command to the PS/2 controller
void ps2_send_command(uint8_t command) {
    while (inb(PS2_CMD_PORT) & 0x02);  // Wait until the controller is ready
    outb(PS2_CMD_PORT, command);
}

// Function to send data to the PS/2 controller
void ps2_send_data(uint8_t data) {
    while (inb(PS2_CMD_PORT) & 0x02);  // Wait until the controller is ready
    outb(PS2_DATA_PORT, data);
}

// Function to read data from the PS/2 controller
uint8_t ps2_read_data() {
    while (!(inb(PS2_CMD_PORT) & 0x01));  // Wait until data is available
    return inb(PS2_DATA_PORT);
}
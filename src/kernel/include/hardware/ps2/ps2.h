#ifndef __PS2__H
#define __PS2__H

#include <stdint.h>

#define PS2_CMD_PORT 0x64
#define PS2_DATA_PORT 0x60

// Function to send a command to the PS/2 controller
void ps2_send_command(uint8_t command) ;

// Function to send data to the PS/2 controller
void ps2_send_data(uint8_t data);

// Function to read data from the PS/2 controller
uint8_t ps2_read_data();

#endif // !__PS2__H

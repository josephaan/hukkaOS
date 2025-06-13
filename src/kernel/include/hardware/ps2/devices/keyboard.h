#ifndef __KEYBOARD__H
#define __KEYBOARD__H


#include <stdint.h>
#include <stddef.h>
#include <hardware/apic/apic.h>


#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

#define KEYBOARD_BUFFER_SIZE 256


void keyboard_interrupt_handler();
void enable_keyboard();
uint8_t get_keyboard_input();



#endif // !1
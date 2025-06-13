#ifndef __CONVERSION__H
#define __CONVERSION__H

// String to str conversion

void int_to_string(int value, char *str);
void long_to_string(int value, char *str);
void pointer_to_hex_string(void* ptr, char* buffer);
void double_to_string(double value, char* buffer, int precision);
void unsigned_long_to_string(unsigned long value, char* buffer);

#endif
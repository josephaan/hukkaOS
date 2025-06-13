#include <debug/log.h>
#include <stdarg.h>
#include <stdbool.h>
#include <conversion.h>
#include <x86_64/cpu.h>

#define SERIAL_PORT_COM1 0x3F8

#ifdef ENABLE_LOGGING
    #define LOG_ENABLED true
#else
    #define LOG_ENABLED false
#endif

static inline void outb(unsigned short port, unsigned char value) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}


void serial_init() {
    if (LOG_ENABLED) {
        outb(SERIAL_PORT_COM1 + 1, 0x00); // Disable all interrupts
        outb(SERIAL_PORT_COM1 + 3, 0x80); // Enable DLAB (set baud rate divisor)
        outb(SERIAL_PORT_COM1 + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
        outb(SERIAL_PORT_COM1 + 1, 0x00); //                  (hi byte)
        outb(SERIAL_PORT_COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
        outb(SERIAL_PORT_COM1 + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
        outb(SERIAL_PORT_COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
    }
}


int serial_is_transmit_empty() {
    return inb(SERIAL_PORT_COM1 + 5) & 0x20;
}

void serial_write_char(char a) {
    while (serial_is_transmit_empty() == 0);
    outb(SERIAL_PORT_COM1, a);
}

void serial_write_string(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        serial_write_char(str[i]);
    }
}

void serial_write_number(uint64_t number){
    char buffer[12];
    unsigned_long_to_string(number, buffer);
    serial_write_string(buffer); 
}


void log(log_level_t level, const char* message, ...) {
    if (LOG_ENABLED) {
        uint32_t cpu_id = get_current_core_id();
        serial_write_string("[CPU ");
        serial_write_number((uint64_t)cpu_id);
        serial_write_string(" ] | ");
        switch (level) {
            case LOG_LEVEL_DEBUG:
                serial_write_string("DEBUG: ");
                break;
            case LOG_LEVEL_INFO:
                serial_write_string("INFO: ");
                break;
            case LOG_LEVEL_WARN:
                serial_write_string("WARN: ");
                break;
            case LOG_LEVEL_ERROR:
                serial_write_string("ERROR: ");
                break;
        }
        va_list args;
        va_start(args, message);
        while (*message) {
            if (*message == '%' && *(message + 1) == 'd') {
                // Handle %d
                int value = va_arg(args, int);
                char num_str[12];
                int_to_string(value, num_str);
                const char *p = num_str;
                while (*p) {
                    serial_write_char(*p);
                    p++;
                }
                message += 2; // Skip %d
            } 
            else if (*message == '%' && *(message + 1) == 'l' ) {
                // handle %l
                if (*(message + 2) == 'u') {
                    unsigned long value = va_arg(args, unsigned long);
                    char num_str[64]; // Find out the precision of float, the 12 is copied from %d
                    unsigned_long_to_string(value, num_str);
                    const char *p = num_str;
                    while (*p) {
                        serial_write_char(*p);
                        p++;
                    }

                    message += 3; //skip %lu
                }         

                else if (*(message + 2) == 'd') {
                    long value = va_arg(args, long);
                    char num_str[64];
                    long_to_string(value, num_str); // 6 decimal places
                    const char *p = num_str;
                    while (*p) {
                        serial_write_char(*p);
                        p++;
                    }

                    message += 3; //skip %ld
                }            

            } else if((*message == '%' && *(message + 1) == 'p')) {
                
                void* value = va_arg(args, void*);
                char num_str[64];
                pointer_to_hex_string(value, num_str);
                const char *p = num_str;
                while (*p) {
                    serial_write_char(*p);
                    p++;
                }

                message += 2; // skip %lf

            }
            else if((*message == '%' && *(message + 1) == 's')) {

                char * string = va_arg(args, char*);
                serial_write_string(string);
                message += 2; // skip %s

            } 
            else if((*message == '%' && *(message + 1) == 'c')){
                char c = (char) va_arg(args, int);
                serial_write_char(c);
                message += 2; // skips %c
            }
            else {
                serial_write_char(*message);
                message++;
            }
        }
        va_end(args);
        serial_write_string(message);
        serial_write_string("\n");
    }
}
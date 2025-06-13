#include <terminal/terminal.h>
#include <conversion.h>
#include <display/framebuffer.h>
#include <stdarg.h>
#include <stdint.h>


void terminal_write(const char *str, ...){
    va_list args;
    va_start(args, str);
    enum color Color = White;
    uint32_t color = Color;

    while (*str) {
        if (*str == '%' && *(str + 1) == 'd') {
            // Handle %d
            int value = va_arg(args, int);
            char num_str[12];
            int_to_string(value, num_str);
            const char *p = num_str;
            while (*p) {
                print_char(*p, color);
                p++;
            }
            str += 2; // Skip %d
        } 
        else if (*str == '%' && *(str + 1) == 'l' ) {
            // handle %l
            if (*(str + 2) == 'u') {
                unsigned long value = va_arg(args, unsigned long);
                char num_str[64]; // Find out the precision of float, the 12 is copied from %d
                unsigned_long_to_string(value, num_str);
                const char *p = num_str;
                while (*p) {
                    print_char(*p, color);
                    p++;
                }
                str += 3; //skip %lu
            }         

            else if (*(str + 2) == 'd') {
                long value = va_arg(args, long);
                char num_str[64];
                long_to_string(value, num_str); // 6 decimal places
                const char *p = num_str;
                while (*p) {
                    print_char(*p, color);
                    p++;
                }

                str += 3; //skip %ld
            } else if (*(str + 2) == 'f') {
                    double value = va_arg(args, double);
                    char num_str[64];
                    double_to_string(value, num_str, 6); // 6 decimal places
                    const char *p  = num_str ;//= num_str;
                    while (*p) {
                        print_char(*p, color);
                        p++;
                    }
                    str += 3; //skip %lf
            }

            

        } else if((*str == '%' && *(str + 1) == 'p')) {
            
            void* value = va_arg(args, void*);
            char num_str[64];
            pointer_to_hex_string(value, num_str);
            const char *p = num_str;
            while (*p) {
                print_char(*p, color);
                p++;
            }

            str += 2; // skip %lf

        }
        else if((*str == '%' && *(str + 1) == 's')) {

            char * string = va_arg(args, char*);
            print_numstr(string, color);
            str += 2; // skip %s

        } 
        else if((*str == '%' && *(str + 1) == 'c')){
            char c = (char) va_arg(args, int);
            print_char(c, color);
            str += 2; // skips %c
        }
         else {
            print_char(*str, color);
            str++;
        }
    }

    va_end(args);

   
}

void terminal_initialize() {
    print_numstr("\nroot@hukkaOS:~/ ", 0xd622);

}
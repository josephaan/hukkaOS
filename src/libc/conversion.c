#include <conversion.h>

// N to string

void int_to_string(int value, char *str) {


    char buffer[12]; // Enough to hold a 32-bit integer as a string
    int pos = 0, neg = 0;
    
    if (value < 0) {
        neg = 1;
        value = -value;
    }

    // Handle 0 explicitly
    if (value == 0) {
        buffer[pos++] = '0';
    } else {
        while (value > 0) {
            buffer[pos++] = '0' + (value % 10);
            value /= 10;
        }
    }

    if (neg) {
        buffer[pos++] = '-';
    }

    // Reverse the string
    int i;
    for (i = 0; i < pos; ++i) {
        str[i] = buffer[pos - 1 - i];
    }
    str[pos] = '\0';

    
}

void unsigned_long_to_string(unsigned long value, char* buffer) {
    char* p = buffer;
    char* start = p;
    do {
        *p++ = '0' + (value % 10);
        value /= 10;
    } while (value);
    *p-- = '\0';

    while (start < p) {
        char temp = *start;
        *start++ = *p;
        *p-- = temp;
    }
}

void long_to_string(int value, char *str) {

    char buffer[64]; // Enough to hold a 64-bit integer as a string
    int pos = 0, neg = 0;
    
    if (value < 0) {
        neg = 1;
        value = -value;
    }

    // Handle 0 explicitly
    if (value == 0) {
        buffer[pos++] = '0';
    } else {
        while (value > 0) {
            buffer[pos++] = '0' + (value % 10);
            value /= 10;
        }
    }

    if (neg) {
        buffer[pos++] = '-';
    }

    // Reverse the string
    int i;
    for (i = 0; i < pos; ++i) {
        str[i] = buffer[pos - 1 - i];
    }
    str[pos] = '\0';

    
}

void double_to_string(double value, char* buff, int precision){
	int pos = 0;
	
	if(value < 0){
			buff[pos] = '-';
			value = -value ; // makes x positive
			pos++;
	}
	
	
	long int_part = (long) value;
	
	char temp_buff[64];
	
	unsigned_long_to_string(int_part, temp_buff);
	const char * p = temp_buff;
	while(*p){
			buff[pos] = *p;
            *p++;
			pos++;
	}
	
	double frac_part = value - (double)int_part;
	buff[pos] ='.';
	pos++; // check if it should come before or after
	double y = 0;
	
	for(int i = 0; i < precision; i++){
			y = frac_part *= 10;
			int n = (int) y;
			buff[pos] = '0' + n;
			pos++;
			frac_part =  y - (double) n;
	
	}
	buff[pos] = '\0';
}

void pointer_to_hex_string(void* ptr, char* buffer) {
    const char* hex_digits = "0123456789ABCDEF";
    unsigned long value = (unsigned long)ptr;
    *buffer++ = '0';
    *buffer++ = 'x';
    for (int i = (sizeof(void*) * 2) - 1; i >= 0; i--) {
        *buffer++ = hex_digits[(value >> (i * 4)) & 0xF];
    }
    *buffer = '\0';
}

// Returns a binary string form of a given unsigned long
// Void uint64_to_bin_string(unsigned long value, char * buffer) {}
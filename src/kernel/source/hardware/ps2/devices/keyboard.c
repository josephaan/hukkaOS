#include <hardware/ps2/devices/keyboard.h>
#include <display/framebuffer.h>
#include <hardware/apic/ioapic.h>
#include <stdbool.h>
#include <debug/log.h>

// Ports for the PIC and keyboard
#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define PIC_EOI 0x20

char keyboard_buffer[KEYBOARD_BUFFER_SIZE];

uint8_t buffer_position = 0;

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(data), "Nd"(port));
}

static uint8_t inb(uint16_t port) {
    unsigned char data = 0;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

// Define the scan code set 1 to ASCII character mapping
char scan_code_to_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
    '9', '0', '-', '=', '\b', /* Backspace */
    '\t', /* Tab */
    'q', 'w', 'e', 'r', /* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
    0, /* 29 - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
    '\0', '`',  0, /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', /* 49 */
    'm', ',', '.', '/',   0, /* Right shift */
    '*',
    0,  /* Alt */
    ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0, 0, 0, 0, 0, 0, 0, 0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
    '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
    '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0, 0, 0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

char shift_set1_scancodes[] = { 
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 
    'O', 'P', '{', '}', '\n', '^', 'A', 'S', 'D', 'F', 'G', 'H', 
    'J', 'K', 'L', ':', '\'', '~', 0, '|', 'Z', 'X', 'C', 'V', 
    'B', 'N', 'M', '<', '>', '?', 0, '*', 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0
};

static uint8_t temp_key = 0;
static bool shift = false;
static bool caps_lock = false;
static bool ctrl = false;

void keyboard_interrupt_handler() {

    size_t scan_code = (size_t)inb(KEYBOARD_DATA_PORT);

    if (scan_code == 0x2A || scan_code == 0x36) 
        shift = true;    

    else if (scan_code == 0x3A) 
        caps_lock ^= true;

    else if (scan_code == 0xBA){
        caps_lock ^= false;
    }    

    else if (scan_code == 0x1D) 
        ctrl = true;   

    else if (scan_code == 0xAA || scan_code == 0xB6)  
        shift = false;  

    else if (scan_code < 0x58) {
        if (scan_code & 0x80) {
            // Key released
            //if ((0x3A & 0x80))  caps_lock = false;
        } else {
            // Key pressed
            if (ctrl) {
                char c = scan_code_to_ascii[scan_code]; // Translate scancode to ASCII character
                temp_key =  c;
                print_numstr("%c", 0xffff, c);
            } else
            if (shift ^ caps_lock) {
                char c = shift_set1_scancodes[scan_code]; // Translate scancode to ASCII character
                temp_key =  c;
                print_numstr("%c", 0xffff, c);
            }
            else {
                char c = scan_code_to_ascii[scan_code]; // Translate scancode to ASCII character
                temp_key =  c;
                print_numstr("%c", 0xffff, c);
            }
        }

    }

    send_eoi(); // Send EOI to APIC

    // Send end-of-interrupt (EOI) signal to the PIC
    //outb(0x20, 0x20);
}


void enable_keyboard(){
    ioapic_set_irq(1, 33, 0);
    ioapic_unmask_irq(33);
    //log(LOG_LEVEL_DEBUG, "IRQ 1 : IDT: 33 Keyboard set");
}

uint8_t get_keyboard_input(){
    return temp_key;
}

void kernel_readline(char* buffer, size_t size) {
    size_t i = 0;
    char c;

    while (i < size - 1) {  // Leave room for null terminator
        c = get_keyboard_input();  // Read a character

        if (c == '\n' || c == '\r') {
            print_char('\n', 0xffffff);  // Echo newline
            break;  // End of line
        } else if (c == '\b' || c == 127) {  // Backspace or Delete
            if (i > 0) {
                i--;
                print_char('\b', 0xffffff);  // Move cursor back
                print_char(' ', 0xffffff);   // Erase character
                print_char('\b', 0xffffff);  // Move cursor back again
            }
        } else if (c >= 32 && c <= 126) {  // Printable ASCII characters
            buffer[i++] = c;
            print_char(c, 0xffffff);  // Echo character
        }
    }

    buffer[i] = '\0';  // Null-terminate the string
}



#include <hardware/ps2/ps2.h>
#include <hardware/ps2/devices/mouse.h>
#include <hardware/apic/ioapic.h>   
#include <hardware/apic/apic.h>  
#include <display/framebuffer.h>
#include <gui/gui.h> 
#include <debug/log.h>

int32_t mouse_x = 0;
int32_t mouse_y = 0;
uint8_t mouse_buttons = 0;

void process_mouse_packet(int8_t mouse_packet[3]);

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(data), "Nd"(port));
}

static uint8_t inb(uint16_t port) {
    unsigned char data = 0;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}


void mouse_wait(uint8_t type) {
    uint32_t time_out = 100000;

    if( type == 0 ) {
        while( time_out-- ) {
            if( (inb(0x64) & 1) == 1 )
                return;
        }
        return;
    } else
    {
        while( time_out-- ) {
            if( (inb(0x64) & 2) == 0 ) {
                return;
            }
        }
        log(LOG_LEVEL_DEBUG,"mouse timed out!\n");
        return;
    }
}

void mouse_write(uint8_t write) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, write);
}

uint8_t mouse_read() {
    mouse_wait(0);
    char c = inb(0x60);
    return c;
}

void enable_mouse() {
    /*// Enable the auxiliary device - mouse
    ps2_send_command(0xA8);
    
    // Enable the mouse device
    ps2_send_command(0xD4);
    ps2_send_data(0xF4);
    */

   uint8_t status;

    mouse_wait(1);
	outb(0x64, 0xA8);
	mouse_wait(1);
	outb(0x64, 0x20);
	mouse_wait(0);
	status = inb(0x60) | 2;
	mouse_wait(1);
	outb(0x64, 0x60);
	mouse_wait(1);
	outb(0x60, status);
	mouse_write(0xF6);
	mouse_read();
	mouse_write(0xF4);
	mouse_read();
    

    ioapic_set_irq(12, 0x23, 0);
    ioapic_unmask_irq(12);
    log(LOG_LEVEL_DEBUG, "IRQ 3: IDT 35 Mouse set");
}



uint8_t mouse_cycle = 0;
int8_t mouse_byte[3];

void mouse_handler() {
    //log(LOG_LEVEL_INFO, "Mouse Triggered");
    uint8_t status = inb(PS2_CMD_PORT);
    while (status & 0x01) {
        int8_t mouse_data = inb(PS2_DATA_PORT);
        switch (mouse_cycle) {
            case 0:
                mouse_byte[0] = mouse_data;
                mouse_cycle++;
                break;
            case 1:
                mouse_byte[1] = mouse_data;
                mouse_cycle++;
                break;
            case 2:
                mouse_byte[2] = mouse_data;
                // Process the mouse packet
                process_mouse_packet(mouse_byte);
                mouse_cycle = 0;
                break;
        }
        status = inb(PS2_CMD_PORT);
    }

    send_eoi();
}

void process_mouse_packet(int8_t mouse_packet[3]) {
    int8_t x_offset = mouse_packet[1];
    int8_t y_offset = -mouse_packet[2]; // Invert Y-axis
    
    mouse_buttons = mouse_packet[0] & 0x07; // Get the button states
    
    // Update mouse position
    mouse_x += x_offset;
    mouse_y += y_offset;
    
    // Clamp mouse position to screen boundaries
    mouse_x = (mouse_x < 0) ? 0 : (mouse_x >= get_screen_width() ? get_screen_width() - 1 : mouse_x);
    mouse_y = (mouse_y < 0) ? 0 : (mouse_y >= get_screen_height() ? get_screen_height() - 1 : mouse_y);
    
    // Call a function to update the GUI based on mouse state
    gui_update_mouse(mouse_x, mouse_y, mouse_buttons);
}
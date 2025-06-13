#include <gui/gui.h>
#include <display/framebuffer.h>
#include <hardware/apic/apic.h>
#include <memory/pmm.h>
#include <string.h>
#include <hardware/ps2/devices/mouse.h>


#define MAX_WINDOWS 10
window_widget_t* windows[MAX_WINDOWS];
uint32_t window_count = 0;
uint64_t* front_buffer;
uint64_t* back_buffer;
extern uint8_t fb;

#define FB_ADDR 0xFFFFFFFFFC000000

void update_screen(uint32_t* buffer){
    memcpy(FB_ADDR, buffer, get_screen_width() * get_screen_height() * sizeof(uint32_t));
    
}

void swap_buffers() {
    memcpy(front_buffer, back_buffer, get_screen_width() * get_screen_height() * sizeof(uint32_t));
    update_screen(front_buffer);  // Implement this function to update your physical framebuffer
}

// gui mouse function

void gui_update_mouse(int32_t x, int32_t y, uint8_t buttons) {

    
    static int32_t old_x = 0, old_y = 0;
    static uint8_t old_buttons = 0;

    // Redraw the area where the old cursor was
    draw_rect(old_x, old_y, 8, 8, 0x000000);

    // Check for window dragging and button clicks
    for (int i = window_count - 1; i >= 0; i--) {
        window_widget_t* win = windows[i];
        if (win->is_dragging) {
            int new_x = x - win->drag_offset_x;
            int new_y = y - win->drag_offset_y;
            
            // Boundary checking
            new_x = (new_x < 0) ? 0 : (new_x + win->width > get_screen_width()) ? get_screen_width() - win->width : new_x;
            new_y = (new_y < 0) ? 0 : (new_y + win->height > get_screen_height()) ? get_screen_height() - win->height : new_y;

            win->x = new_x;
            win->y = new_y;
            draw_window(win);
            break;
        } else if (x >= win->x && x < win->x + win->width && y >= win->y && y < win->y + win->height) {
            if ((buttons & 1) && !(old_buttons & 1)) { // Left button just pressed
                if (y < win->y + 20) { // Title bar
                    win->is_dragging = true;
                    win->drag_offset_x = x - win->x;
                    win->drag_offset_y = y - win->y;
                } else if (x >= win->close_button.x && x < win->close_button.x + win->close_button.width &&
                           y >= win->close_button.y && y < win->close_button.y + win->close_button.height) {
                    // Close button clicked
                    // Implement window closing logic here
                    //win->is_closed = true;
                }
            }
            break;
        }
    }

    // Release drag if button is released
    if (!(buttons & 1) && (old_buttons & 1)) {
        for (int i = 0; i < window_count; i++) {
            windows[i]->is_dragging = false;
        }
    }

    // Draw the new cursor
    draw_cursor(x, y);

    old_x = x;
    old_y = y;
    old_buttons = buttons;
}

// Simple arrow cursor
void draw_cursor(int32_t x, int32_t y) {
    draw_rect(x, y, 8, 8, 0xFFFFFF);
    draw_rect(x, y, 7, 7, 0x000000);
}

void draw_window(window_widget_t* win) {
    if(win->is_closed == false){
        // Draw window title bar
        uint32_t title_bar_height = 20;
        uint32_t title_color = 0x0000FF; // Blue title bar
        draw_rect(win->x, win->y, win->width, title_bar_height, title_color);

        // Draw window title text
        uint32_t text_color = 0xFFFFFF; // White text
        draw_string(win->x + 10, win->y + 5, win->title, text_color);

        
        // Initialize close button
            win->close_button = (Button){
                .x = win->x + win->width - 25,
                .y = win->y + 5,
                .width = 20,
                .height = 20,
                .text = "X",
                .color = 0xFF0000,
                .text_color = 0xFFFFFF
            };
        draw_button(&win->close_button);

        // Draw window background
        draw_win_rect(win->x, win->y, win->width, win->height, win->background_color, title_bar_height);

        // Draw window borders
        uint32_t border_color = 0xFFFFFF; // White borders
        for (uint32_t i = 0; i < win->height; i++) {
            put_pixel(win->x, win->y + i, border_color); // Left border
            put_pixel(win->x + win->width - 1, win->y + i, border_color); // Right border
        }
        for (uint32_t i = 0; i < win->width; i++) {
            put_pixel(win->x + i, win->y, border_color); // Top border
            put_pixel(win->x + i, win->y + win->height - 1, border_color); // Bottom border
        }
    }
}


void create_window(char * window_title, 
                    uint32_t width, 
                    uint32_t height
                    ){
            
            if (window_count >= MAX_WINDOWS) return;
    
            window_widget_t* new = allocate_memory(sizeof(window_widget_t));
            new->background_color = 0x00000f;
            new->height = height;
            new->width = width;
            strncpy(new->title, window_title, 126);
            new->title[127] = '\0';
            new->x = 10 + (window_count * 20); // Stagger new windows
            new->y = 10 + (window_count * 20);
            new->is_dragging = false;

            // Initialize close button
            new->close_button = (Button){
                .x = new->x + new->width - 25,
                .y = new->y + 5,
                .width = 20,
                .height = 20,
                .text = "X",
                .color = 0xFF0000,
                .text_color = 0xFFFFFF
            };

            new->is_closed = false;
            
            windows[window_count++] = new;
            draw_window(new);



}


void draw_desktop(){
    clear_fb();
    //enum color Color = Dark_Gray;
    //clear_framebuffer(Color);
}


void draw_all_windows() {
    for (uint32_t i = 0; i < window_count; i++) {
        draw_window(windows[i]);
    }
}


void init_gui(){
    
    while(1)
    {
        draw_desktop();
        draw_all_windows();
        gui_update_mouse(30,30, 0);
        enable_mouse();
        // You might want to add a small delay here to avoid excessive redrawing
        sleep(1000); // ~60 FPS
    }
    
    
}

/*
void init_gui() {
    front_buffer = FB_ADDR;  // Implement this function to get your framebuffer address
    back_buffer = allocate_memory(get_screen_width() * get_screen_height() * sizeof(uint32_t));
    // Clear back buffer
    memset(back_buffer, 0, get_screen_width() * get_screen_height() * sizeof(uint32_t));

    while (1) {
        

        gui_update_mouse(30, 30, 0);
        draw_all_windows();

        // Swap buffers
        swap_buffers();

        sleep(16); // ~60 FPS
    }
}

*/
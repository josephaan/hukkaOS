#ifndef __GUI__H
#define __GUI__H

#include <stdint.h>
#include <stdbool.h>
#include <gui/widgets.h>

typedef struct window_widget{
    uint32_t x, y, width, height;
    uint32_t background_color;
    char title[128];
    bool is_dragging;
    int drag_offset_x, drag_offset_y;
    Button close_button;
    bool is_closed;
    
} window_widget_t;

void draw_window(window_widget_t* win);

void create_window(char * window_title, uint32_t width, uint32_t height );

void gui_update_mouse(int32_t x, int32_t y, uint8_t buttons);
void draw_cursor(int32_t x, int32_t y);

void init_gui();

#endif // !__GUI__H

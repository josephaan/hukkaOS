#ifndef __GUI__WIDGETS__H
#define __GUI__WIDGETS__H

#include <stdint.h>

typedef struct {
    int x, y, width, height;
    const char* text;
    uint32_t color;
    uint32_t text_color;
} Button;

typedef struct {
    int x, y, width, height;
    char* text;
    int text_length;
    uint32_t color;
    uint32_t text_color;
} TextField;

void draw_button(Button* button);

void draw_text_field(TextField* field);

void draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void draw_win_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color, uint32_t title_bar_height);

void draw_string(uint32_t x, uint32_t y, const char* str, uint32_t color);

#endif // !__GUI_WIDGETS__H
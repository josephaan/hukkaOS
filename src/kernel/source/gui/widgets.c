#include <gui/widgets.h>
#include <display/framebuffer.h>
#include <string.h>

// Writes string in gui window
void draw_string(uint32_t x, uint32_t y, const char* str, uint32_t color) {
    while (*str) {
        draw_char(x, y, *str++, color);
        x += 8; // Assuming fixed-width font
    }
}


void draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    for (uint32_t i = 0; i < h; i++) {
        for (uint32_t j = 0; j < w; j++) {
            put_pixel(x + j, y + i, color);
        }
    }
}

void draw_win_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color, uint32_t title_bar_height) {
    for (uint32_t i = 0; i < h; i++) {
        if(i > (title_bar_height - 1)){
            for (uint32_t j = 0; j < w; j++) {
                put_pixel(x + j, y + i, color);
            }
        }
    }
}

void draw_button(Button* button) {
    draw_rect(button->x, button->y, button->width, button->height, button->color);
    int text_x = button->x + (button->width - strlen(button->text) * 8) / 2;
    int text_y = button->y + (button->height - 8) / 2;
    draw_string(text_x + 5, text_y, button->text, button->text_color);
}

void draw_text_field(TextField* field) {
    draw_rect(field->x, field->y, field->width, field->height, field->color);
    draw_string(field->x + 8, field->y + 5, field->text, field->text_color);
}
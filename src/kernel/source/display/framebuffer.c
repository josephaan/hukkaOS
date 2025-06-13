/*
 * framebuffer.c
 * 
 *
 * Copyright (C) 2024 - 2026 Joseph Haita (haitajoseph2@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the HukkaOS. 
 * 
 *
 */


#include <display/framebuffer.h>
#include <font8x8_basic.h>
#include <bootboot.h>
#include <conversion.h>
#include <logos/kyle.h>
#include <debug/log.h>

 
/* imported virtual addresses, see linker script */

extern BOOTBOOT bootboot;               // see bootboot.h
extern unsigned char environment[4096]; // configuration, UTF-8 text key=value pairs
extern uint8_t fb;                      // linear framebuffer mapped The frame buffer pointer


/**************************
 * Display text on screen *
 **************************/


extern volatile uint8_t _binary_src_fonts_font_psf_start;

// Track the cursor position
uint32_t cursor_x = 0;
uint32_t cursor_y = 0;
const uint32_t char_width = 8;
const uint32_t char_height = 8;




static hukaos_framebuffer_t framebuffer_details;

void initialize_framebuffer_details(){
    framebuffer_details.height = bootboot.fb_height;
    log(LOG_LEVEL_DEBUG, "framebuffer height = %lu", framebuffer_details.height);
    framebuffer_details.width = bootboot.fb_width;
    log(LOG_LEVEL_DEBUG, "framebuffer width = %lu", framebuffer_details.width);
    framebuffer_details.scanline = bootboot.fb_scanline;
    framebuffer_details.type = bootboot.fb_type;
}


uint32_t get_screen_width(){
    // In the future the height should be stored in a seperate structure to avoid dependency on the bootloader
    return framebuffer_details.width;
}

uint32_t get_screen_height(){
    return framebuffer_details.height;
}


// Enhanced puts function to handle \n and %d
void disp_str(const char *s, ...) {
    psf2_t *font = (psf2_t*)&_binary_src_fonts_font_psf_start;
    uint32_t x, y, kx = 0, line, mask, offs;
    uint32_t bpl = (font->width + 7) / 8;

    va_list args;
    va_start(args, s);

    while (*s) {
        if (*s == '\n') {
            kx = 0;  // Reset horizontal position
            cursor_y += font->height;  // Move down by one character height
            if (cursor_y >= bootboot.fb_height) {
                cursor_y = 0;  // Wrap vertically if needed
            }
            s++;
            continue;
        } else if (*s == '%' && *(s + 1) == 'd') {
            int value = va_arg(args, int);
            char num_str[12];
            int_to_string(value, num_str);
            for (char *p = num_str; *p != '\0'; p++) {
                unsigned char *glyph = (unsigned char*)&_binary_src_fonts_font_psf_start + font->headersize +
                    (*p > 0 && (uint32_t)*p < font->numglyph ? *p : 0) * font->bytesperglyph;
                offs = (kx * (font->width + 1) * 4) + cursor_y * bootboot.fb_scanline;
                for (y = 0; y < font->height; y++) {
                    line = offs; mask = 1 << (font->width - 1);
                    for (x = 0; x < font->width; x++) {
                        *((uint32_t*)((uint64_t)&fb + line)) = ((int)*glyph) & (mask) ? 0xFFFFFF : 0;
                        mask >>= 1; line += 4;
                    }
                    *((uint32_t*)((uint64_t)&fb + line)) = 0; glyph += bpl; offs += bootboot.fb_scanline;
                }
                kx++;
            }
            s += 2;  // Skip "%d"
            continue;
        } else {
            unsigned char *glyph = (unsigned char*)&_binary_src_fonts_font_psf_start + font->headersize +
                (*s > 0 && (uint32_t)*s < font->numglyph ? *s : 0) * font->bytesperglyph;
            offs = (kx * (font->width + 1) * 4) + cursor_y * bootboot.fb_scanline;
            for (y = 0; y < font->height; y++) {
                line = offs; mask = 1 << (font->width - 1);
                for (x = 0; x < font->width; x++) {
                    *((uint32_t*)((uint64_t)&fb + line)) = ((int)*glyph) & (mask) ? 0xFFFFFF : 0;
                    mask >>= 1; line += 4;
                }
                *((uint32_t*)((uint64_t)&fb + line)) = 0; glyph += bpl; offs += bootboot.fb_scanline;
            }
            kx++;
        }
        s++;
    }

    va_end(args);
}

void put_pixel(uint32_t x, uint32_t y, uint32_t color) {

    uint32_t offset =  (8 + y) * bootboot.fb_scanline + 4 * (x + 8) ; 
    uint32_t *pixels = (uint32_t *)(&fb + offset);
    *pixels = color;
}

void draw_char(uint32_t x, uint32_t y, char c, uint32_t color) {

    for (int i = 0; i < 8; ++i) {  // 8 rows per character
        uint8_t row = font8x8_basic[(unsigned char)c][i]; 
        for (int j = 0; j < 8; ++j) {  // 8 columns per row
            if (row & (1 << j)) { //Reverse bit  order
                put_pixel(x - (7 - j), y + i, color); // x  + (7 - j) will invert the lettter, or create mirror image of letter
            }
        }
    }
    
}

void print_char(char c, uint32_t color) {

    if (c == '\n') {
        cursor_x = 0;
        cursor_y += char_height;
        if (cursor_y >= bootboot.fb_height) {
            cursor_y = 0;  // Wrap around vertically if needed
            scroll();
        }

    } else {
        if (cursor_x + char_width > bootboot.fb_width) {
            cursor_x = 0;
            cursor_y += char_height;
            if (cursor_y >= bootboot.fb_height) {
                cursor_y = 0;  // Wrap around vertically if needed
                scroll();
            }
        }
        draw_char(cursor_x, cursor_y, c, color);
        cursor_x += char_width;
    }
    
}

void print_numstr(const char *str, uint32_t color, ...) {

    va_list args;
    va_start(args, color);

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

        } else if((*str == '%' && *(str + 1) == 'c')){
            char c = (char) va_arg(args, int);
            print_char(c, color);
            str += 2; // skips %c
        }   else {
            print_char(*str, color);
            str++;
        }
    }

    va_end(args);
    
    
}

void clear_framebuffer(uint32_t color) {

    uint32_t j, y, x, i;
    // clear the screen
        for(j=y=0;y<bootboot.fb_height;y++) {
            i=j;
            for(x=0;x<bootboot.fb_width;x+=2,i+=8)
                *((uint64_t*)(bootboot.fb_ptr + i))=color;
            j+=bootboot.fb_scanline;
        }
}
    

void clear_screen(uint32_t color){

    // Clear the framebuffer of any characters whilst fealing it with a given color

    uint32_t offset =  (bootboot.fb_height) * bootboot.fb_scanline;
    uint32_t *pixels = (uint32_t *)(&fb + offset);
    *pixels = color;

}

void clear_fb(){
    uint32_t j, y, x, i;
    // clear the screen
    for(j=y=0;y<bootboot.fb_height;y++) {
        i=j;
        for(x=0;x<bootboot.fb_width;x+=2,i+=8)
            *((uint64_t*)(bootboot.fb_ptr + i))=0;
        j+=bootboot.fb_scanline;
    }
}

void scroll() {
    // Move all rows up by one character height
    for (uint32_t y = 0; y < bootboot.fb_height - char_height; y++) {
        uint32_t src_offset = (y + char_height) * bootboot.fb_scanline;
        uint32_t dest_offset = y * bootboot.fb_scanline;
        uint32_t *src = (uint32_t *)((uint64_t)&fb + src_offset);
        uint32_t *dest = (uint32_t *)((uint64_t)&fb + dest_offset);
        for (uint32_t x = 0; x < bootboot.fb_width; x++) {
            dest[x] = src[x];
        }
    }

    // Clear the bottom rows
    for (uint32_t y = bootboot.fb_height - char_height; y < bootboot.fb_height; y++) {
        for (uint32_t x = 0; x < bootboot.fb_width; x++) {
            put_pixel(x, y, 0x000000); // Assuming black as the background color
        }
    }

    cursor_y -= char_height;
}




void draw_kyle() {

    int bytesPerPixel = 1;
    int index = 0;
    for (int y = 0; y < BITMAP_HEIGHT; y++) {
        for (int x = 0; x < BITMAP_WIDTH; x++) {
            uint32_t color = 0;
            
            // Construct the color based on bytes per pixel
            switch (bytesPerPixel) {
                case 1: // Grayscale
                    color = (bitmap[index] << 16) | (bitmap[index] << 8) | bitmap[index];
                    index += 1;
                    break;
                case 3: // RGB
                    color = (bitmap[index] << 16) | (bitmap[index + 1] << 8) | bitmap[index + 2];
                    index += 3;
                    break;
                case 4: // RGBA
                    color = (bitmap[index] << 24) | (bitmap[index + 1] << 16) | 
                            (bitmap[index + 2] << 8) | bitmap[index + 3];
                    index += 4;
                    break;
                default:
                    return;
            }
            
            put_pixel(x, y, color);
        }
    }
}
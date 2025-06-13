/*
 * framebuffer.h
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


#ifndef __FRAMEBUFFER__H
#define __FRAMEBUFFER__H

#include <stdint.h>
#include <stdarg.h>

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
    uint8_t glyphs;

} __attribute__((packed)) psf2_t;


typedef struct hukaos_framebuffer{
    uint32_t width;
    uint32_t height;
    uint32_t scanline;
    uint8_t type;
} hukaos_framebuffer_t;


enum color {
    Black        = 0x000000,
    Blue         = 0x0000AA,
    Green        = 0x00AA00,
    Cyan         = 0x00AAAA,
    Red          = 0xAA0000,
    Purple       = 0xAA00AA,
    Brown        = 0xAA5500,
    Gray         = 0xAAAAAA,
    Dark_Gray    = 0x555555,
    Light_Blue 	 = 0x5555FF,
    Light_Green  = 0x55FF55,
    Light_Cyan 	 = 0x55FFFF,
    Light_Red 	 = 0xFF5555,
    Light_Purple = 0xFF55FF,
    Yellow 	     = 0xFFFF55,
    White        = 0xFFFFFF 
} ;

// Init function

void initialize_framebuffer_details();


void put_pixel(uint32_t x, uint32_t y, uint32_t color);
void draw_char(uint32_t x, uint32_t y, char c, uint32_t color);
void print_char(char c, uint32_t color);

//void print_string(const char *str, uint32_t x, uint32_t y, uint32_t color);

void print_numstr(const char *str, uint32_t color, ...) ;

void clear_framebuffer(uint32_t color);
void clear_screen(uint32_t color);
void clear_fb();

void disp_str(const char *s, ...);
void draw_logo(uint32_t x, uint32_t y, uint32_t color);
void draw_kyle();

void scroll();

uint32_t get_screen_width();

uint32_t get_screen_height();

#endif // FRAMEBUFFER



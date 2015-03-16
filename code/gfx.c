/* Functions for a simple graphics library for the bare metal
 * Raspberry Pi.
 
 * Author: Rishi Bedi <rbedi@stanford.edu>
 * Date: Feb 25 2015
 */ 


#include "gfx.h"
#include "fb.h"
#include "font.h"

/**
 * Wrapper for the same fb method.
 */
int gfx_set_double_buffering(int status){
    return fb_set_double_buffering(status);
}

/**
 * Wrapper for the same fb method.
 */
int gfx_check_double_buffering(){
    return fb_check_double_buffering();
}

/**
 * Initializes framebuffer using fb_init()
 */
unsigned int gfx_init() {
    return fb_init();
}

/**
 * Returns pointer to writeable buffer.
 */
volatile char* gfx_buffer(){
    return fb_buffer();
}

/**
 * Clears invisible buffer
 */ 
void gfx_clear(){
    fb_clear();
}

/**
 * Swaps invisible and visible buffers.
 */
void gfx_draw(){
    fb_draw();
}

/**
 * Given RGB bytes, computes a 32-bit little-endian int of the 
 * following format: 0xAABBGGRR, where AA is the alpha value and is set to FF.
 */
unsigned int gfx_compute_color(unsigned char red,
                               unsigned char green,
                               unsigned char blue) {

    return (0xFF << 24) | (blue << 16) | (green << 8) | red;

}

/** 
 * Returns actual width of display.
 */
unsigned int gfx_get_width() {
    return fb_width();
}

/**
 * Returns actual height of display.
 */
unsigned int gfx_get_height() {
    return fb_height();
}

/**
 * Plots a single pixel of color color at position x,y 
 */
void gfx_plot(unsigned int color,
              unsigned int x,
              unsigned int y) {

    volatile unsigned int* buffer;
    if(gfx_check_double_buffering()){ 
          buffer = (unsigned int*)fb_buffer();
    }
    else{
        buffer = (unsigned int*)fb_display();
    }
    buffer[y * gfx_get_width() + x] = color;
    return;
}


/**
 * Returns height of single letter
 */
unsigned int gfx_letter_height() {
    return font_height();
}

/**
 * Returns width of single letter
 */
unsigned int gfx_letter_width() {
    return font_width();
}

/**
 * Fills entire display with color color.
 */
void gfx_fill(unsigned int color){
    volatile int x;
    volatile int y;
    for(x = 0; x < gfx_get_width(); x++){
        for(y = 0; y < gfx_get_height(); y++){
            gfx_plot(color, x, y);
        }
    }

}

/**
 * Draws single character with color color, with its top left
 * at position x,y. Truncates if it doesn't fit in on either right side
 * or bottom.
 */
unsigned int gfx_draw_letter(unsigned int color,
                             unsigned int x,
                             unsigned int y,
                             char letter) {

    int bufLen = font_buflen();
    char buf[bufLen];
    font_ascii(letter, buf, bufLen);
    volatile unsigned int* int_buf = (unsigned int*)buf;
    volatile int fontX;
    volatile int fontY;
   
    int max_width = x + font_width() > gfx_get_width() ? gfx_get_width() : x + font_width();
    int max_height = y + font_height() > gfx_get_height() ? gfx_get_height() : y + font_height();
    
    for(fontX = x; fontX < max_width; fontX++){
        for(fontY = y; fontY < max_height; fontY++){
            unsigned int font_pixel = int_buf[(fontY - y) * font_width() + (fontX - x)];
            unsigned char font_scalar = font_pixel & (0xFF);
          //  if((font_pixel & 0xFF) != 255){
                unsigned char new_red = (font_scalar * (color & 0xFF)) / 255;
                unsigned char new_green = (font_scalar * ((color >> 8) & 0xFF)) / 255;
                unsigned char new_blue = (font_scalar * ((color >> 16) & 0xFF)) / 255;
                unsigned int new_color = gfx_compute_color(new_red, new_green, new_blue);
                gfx_plot(new_color, fontX, fontY);
          //  }
        }
    }
    
    return 1;
}
 
/**
 * Draws string with top left corner at pos x,y
 * Truncates if doesn't fit on bottom or right.
 */
unsigned int gfx_draw_string(unsigned int color,
                             unsigned int x,
                             unsigned int y,
                             char* str) {

   int curr_start_x = x;

   while(*str){
       gfx_draw_letter(color, curr_start_x, y, *str); 
       curr_start_x += gfx_letter_width();       
       str++;
   } 
   return 1;

}



/*
 * Copyright (c) 2014 Stanford University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the Stanford University nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL STANFORD
 * UNIVERSITY OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

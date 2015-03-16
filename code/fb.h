#ifndef FB_H_INCLUDED
#define FB_H_INCLUDED


#define DOUBLE_BUFFERING 1

#if DOUBLE_BUFFERING

void fb_clear();
void fb_draw();

#endif

int fb_check_double_buffering();
int fb_set_double_buffering(int status);

unsigned int fb_init();

/* Return a pointer to the off-screen buffer. */

volatile char* fb_buffer();
volatile char* fb_display();

unsigned int fb_byte_depth();
unsigned int fb_height();
unsigned int fb_width();

/* Clear the off-scren buffer.*/
void fb_clear();

/* Swap the on-screen and off-screen buffers. */
void fb_draw();

#endif

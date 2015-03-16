#include "fb.h"
#define MAILBOX_BASE 0x2000B880
#define GPU_OFFSET 0x40000000

#define DISPLAY_HEIGHT 960
#define DISPLAY_WIDTH 1280

volatile char* writeable_buffer;
volatile char* visible_buffer;

typedef struct {
    int width;
    int height;
    int virtual_width;
    int virtual_height;
    int pitch;
    int depth;
    int x_offset;
    int y_offset;
    int pointer;
    int size;
} fb_config_t;

fb_config_t config __attribute__ ((aligned(16)));



typedef struct {
    int read;
    int pad1[3];
    int peek;
    int sender;
    int status;
    int configuration;
    int write;
} fb_mailbox;

static volatile int current_offset = 0;
static volatile int double_buffering = 0;

/**
 *
 * if status == 1, sets double buffering on
 * if status == 0, turns double buffering off
 *
 */
int fb_set_double_buffering(int status){
    double_buffering = status;
    return 0;
}


/**
 * Returns 0 is double buffering is off, and 1 if 
 * double buffering is on.
 *
 */
int fb_check_double_buffering(){
    return double_buffering;
}

/**
 * Initializes framebuffer.
 */
unsigned int fb_init(){

    volatile fb_mailbox *mail = (fb_mailbox *)(MAILBOX_BASE);
    while( mail->status & (1 << 31) ){ 
        // do nothing        
    }     

    config.width = fb_width();
    config.height = fb_height();
    config.virtual_width = fb_width();
    
    if(fb_check_double_buffering()){
        config.virtual_height = fb_height() * 2;
    }
    else{
        config.virtual_height = fb_height();
    }

    config.depth = fb_byte_depth() * 8;
    config.x_offset = 0;
    config.y_offset = 0;

   mail->write = (unsigned int)&config + GPU_OFFSET + 1;


   while(1){
       while( ((mail->status) & (1 << 30)) ){
                // do nothing
       }

       if ((mail->read & 0xF) == 0x1){
            break;
       }
   }
   visible_buffer = (char *)config.pointer;
   writeable_buffer = visible_buffer + (fb_width() * fb_height() * fb_byte_depth());
   return 0;
}


/**
 * In the double buffer implementation, clears the invisible buffer.
 */
void fb_clear(){
   
    int bytes_to_clear = fb_width() * fb_height() * fb_byte_depth() / 8;
    volatile unsigned long long* mem_to_erase = (unsigned long long*) writeable_buffer;
    *mem_to_erase = 0;
    while(bytes_to_clear--){
        *(mem_to_erase++) = 0;
    }

}

/**
 * In the double buffer implementation, swaps the writeable and displayed buffers.
 */
void fb_draw(){

    volatile fb_mailbox *mail = (fb_mailbox *)(MAILBOX_BASE);
    while( mail->status & (1 << 31) ){ 
        // do nothing        
    }     
    
    
    if(current_offset == 0){
        config.y_offset = fb_height();
        current_offset = 1;
    }
    else{
        config.y_offset = 0;
        current_offset = 0;
    }

    volatile char* tmp_buffer = writeable_buffer;
    writeable_buffer = visible_buffer;
    visible_buffer = tmp_buffer;
        
    mail->write = (unsigned int)&config + GPU_OFFSET + 1;


    while(1){
       while( ((mail->status) & (1 << 30)) ){
                // do nothing
       }

       if ((mail->read & 0xF) == 0x1){
            break;
       }
    }
    
    
    return;
}


/**
 * Returns writeable (invisible) buffer in the double buffer implementation.
 */
volatile char* fb_buffer(){
    return writeable_buffer;
}

/**
 * Returns visible buffer in both implementations.
 */
volatile char* fb_display(){
    return visible_buffer;
}

/**
 * Returns number of bytes per pixel.
 * Default: 4 (RGBA)
 */
unsigned fb_byte_depth(){
    return 4;
}

/** 
 * Returns height of display in pixels.
 */
unsigned fb_height(){
    return DISPLAY_HEIGHT;
}

/**
 * Returns width of display in pixels.
 */
unsigned fb_width(){
    return DISPLAY_WIDTH;
}

#include "system.h"
#include "led.h"
#include "timer.h"
#include "gpio.h"
#include "fb.h"
#include "gfx.h"
#include "keyboard.h"
#include "rpi_logo.h"
#include "uart.h"
#include "printf.h"
#include "utils.h"
#include "console.h"

#define MAX_COMMAND_LENGTH 100
#define FONT_COLOR 0xFFFFFFFF
#define min(a,b)(a < b ? a : b);
#define max(a,b)(a > b ? a : b);


typedef struct{
    int num_lines;
    int current_line;
    int num_chars_per_line;
    int clear_needed;
    int cur_char_pos;
    int last_line_drawn;
    char (*lines)[MAX_COMMAND_LENGTH];
} console;


console con;

void draw_console(console* con, int clear){
    
    int i;
    if(clear){
        con->last_line_drawn = 0;
        gfx_clear();
    }
    for(i = con->last_line_drawn; i <= con->current_line; i++){
        gfx_draw_string(FONT_COLOR, 0, i * gfx_letter_height(), con->lines[i]);
    }
    
    gfx_draw_letter(0xFF0000FF, gfx_letter_width() * con->cur_char_pos, gfx_letter_height() * con->current_line, '_'); 

    gfx_draw();
    if(clear){    
        gfx_clear();
    }

    for(i = con->last_line_drawn; i <= con->current_line; i++){
        gfx_draw_string(FONT_COLOR, 0, i * gfx_letter_height(), con->lines[i]);
    }
    gfx_draw_letter(0xFF0000FF, gfx_letter_width() * con->cur_char_pos, gfx_letter_height() * con->current_line, '_'); 
    con->last_line_drawn = con->current_line;
    con->clear_needed = 0;
}


void console_clear(console* con){
    int i;
    for(i = 0; i < con->num_lines; i++){
        con->lines[i][0] = '\0';
    }
    con->current_line = 0;
    draw_console(con, 1);
    return; 
}

void console_logo(console* con){
    
   gfx_clear();
   gfx_draw();
   gfx_clear();
   volatile char* output_buffer = gfx_buffer();
   char* image_buffer = rpi_logo_image();
   int left_offset = (gfx_get_width() - rpi_logo_width())/2;
   int top_offset = (gfx_get_height() - rpi_logo_height())/2;
   int y;
   for(y = 0; y < rpi_logo_height(); y++){
        imgcpy(&output_buffer[((top_offset + y) * gfx_get_width() + left_offset) * rpi_logo_byte_depth()], &image_buffer[(y * rpi_logo_width()) * rpi_logo_byte_depth()], rpi_logo_byte_depth() * rpi_logo_width()); 
   }
  con->clear_needed = 1;
  gfx_draw();
}


int console_execute(char* command, console* con){
    if(strcmp(command,"list") == 0){
       console_print("list");
       console_print("clear");
       console_print("logo");
    }
    else if(strcmp(command,"logo") == 0){
        console_logo(con);
    }
    else if(strcmp(command,"clear") == 0){
        console_clear(con); 
    }
    else{
        console_print("Not a valid command.");
    }
    return 0;
}


void init(){
  gpio_init();
  led_init();
  timer_init();
  fb_init();
  keyboard_init();
  uart_init();
  system_enable_interrupts();
   
  gfx_set_double_buffering(1);
  gfx_init();

  /** Ensure everything is zeroed out so we start with a black screen **/
  gfx_clear();
  gfx_draw();
  gfx_clear();
}


void clear_cursor(console* con){
   int x = gfx_letter_width() * con->cur_char_pos;
   int y = con->current_line * gfx_letter_height();
   gfx_draw_letter(0xFF000000, x, y, ' ');
   gfx_draw(); 
   gfx_draw_letter(0xFF000000, x, y, ' ');
}

void console_print(char* str){
   if((con.current_line + 1) >= con.num_lines){
        console_clear(&con);
   }
   strcpy((char*)(con.lines[con.current_line]), str);
   con.current_line++;
   draw_console(&con, con.clear_needed); 
}

void notmain() {
  init();
  con.num_lines = (gfx_get_height() / gfx_letter_height());
  int max_chars_on_display = gfx_get_width() / gfx_letter_width();
  con.num_chars_per_line = min(MAX_COMMAND_LENGTH, max_chars_on_display);
  con.current_line = 0;
  char lines[con.num_lines][MAX_COMMAND_LENGTH];
  con.lines = lines;
  con.last_line_drawn = 0;
  con.cur_char_pos = 0;

  while (1) {
       
    if(keyboard_has_char()){
        char next_ch = keyboard_read_char();      
        uart_putc(next_ch);        
        if(next_ch == 0xD || next_ch == 0xA){
            if((con.current_line + 1) < con.num_lines){
                clear_cursor(&con);
                (con.current_line)++;
            }
            else{
                console_clear(&con);
            }
            con.cur_char_pos = 0;
            console_execute(con.lines[con.current_line-1], &con);
        }
        else if(next_ch == 0x8){
            if(con.cur_char_pos > 0){
                clear_cursor(&con);
                con.cur_char_pos--;
                con.lines[con.current_line][con.cur_char_pos] = ' '; 
                con.lines[con.current_line][con.cur_char_pos+1] = '\0';
                draw_console(&con, con.clear_needed);
            }
        }
        else if(con.cur_char_pos + 1 < con.num_chars_per_line){
            con.lines[con.current_line][con.cur_char_pos] = next_ch;
            con.lines[con.current_line][con.cur_char_pos+1] = '\0';
            con.cur_char_pos++; 
            draw_console(&con, con.clear_needed);
        }
    }

  }
}


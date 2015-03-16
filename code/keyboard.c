#include "keyboard.h"
#include "system.h"
#include "gpio.h"
#include "uart.h"
#include "ps2.h"

/* Don't forget to mark any variables touched by interrupts as volatile! */

/* This file should define an interrupt handler for when there is a 
   falling edge on GPIO pin 23. It should read GPIO pin 24 in that
   handler to read in a bit of a PS2 scan code. If it successfully
   receives a scan code, the code should be put into a ring buffer.
   A call to keyboard_has_char() removes scan codes from the ring
   buffer until no remain (returning 0) or a character is read
   (storing the character and returning 1). */

#define INTERRUPT_ENABLE_1  0x2000b210
#define INTERRUPT_ENABLE_2  0x2000b214
#define INTERRUPT_DISABLE_1 0x2000b21c
#define INTERRUPT_DISABLE_2 0x2000b220

#define GPLEV0              0x20200034
#define GPLEV1              0x20200038
#define GPEDS0              0x20200040
#define GPEDS1              0x20200044



unsigned int buffer_has_elements();
void buffer_insert(unsigned int to_insert);
unsigned int buffer_remove();

volatile int current_bit_index = 0;
volatile unsigned int buffer[128];
volatile int current_scan_code = 0;
volatile int buffer_head = 0;
volatile int buffer_tail = 0;


volatile char last_char_read = 'a';
volatile int shift_pressed = 0;
volatile int caps_lock_enabled = 0;


/* Return 1 if there are characters to read, 0 if no characters to read.
   This call should process PS2 scan codes in the ring buffer, returning
   after the first scan code that results in a pressed character. This
   keeps the interrupt handler simple (just put scan codes in ring
   buffer) and puts the complex logic (handling shift, caps lock, etc.)
   in this call.

   This call should not assume that it is followed by keyboard_read_char().
   That is, if a program calls keyboard_has_chars() twice, this should
   not result in characters being lost.*/
unsigned keyboard_has_char() {
    while(1){
        if(!buffer_has_elements()){
            return 0;
        }
        unsigned int read_char = buffer_remove();
     
        if(read_char == KEYBOARD_CODE_SHIFTL || read_char == KEYBOARD_CODE_SHIFTR){ 
            if(last_char_read == KEYBOARD_CODE_RELEASE){
                last_char_read = read_char;
                shift_pressed = 0;
                continue;
            }
            else{
                last_char_read = read_char;
                shift_pressed = 1;
                continue;
            }
        }

        if(read_char == KEYBOARD_CODE_CAPS_LOCK && last_char_read == KEYBOARD_CODE_RELEASE){
            caps_lock_enabled ^= 1;
            last_char_read = read_char;
            continue;
        }

        if(last_char_read == KEYBOARD_CODE_RELEASE || read_char == KEYBOARD_CODE_VAR_IGNORE || read_char == KEYBOARD_CODE_CAPS_LOCK || read_char == KEYBOARD_CODE_RELEASE){
            last_char_read = read_char;
            continue;
        }

        last_char_read = read_char;

        return 1;
    }
}

/* Return the next ASCII character pressed. If keyboard_has_char() is
   false, the return value of this function is undefined. This
   function can assume that keyboard_has_char() was called before it
   and that call returned true. This call does not block. */
char keyboard_read_char() {

        if((shift_pressed && !caps_lock_enabled) || (!shift_pressed && caps_lock_enabled)){
            return shift_lookup_table[last_char_read];
        }
        return lookup_table[last_char_read];
}



void buffer_insert(unsigned int to_insert){
   int next_tail = (buffer_tail + 1) % RING_BUFFER_SIZE;
   if(next_tail == buffer_head) {
        return;
   }
   buffer[next_tail] = to_insert;
   buffer_tail = next_tail;
}

unsigned int buffer_has_elements(){
    return (buffer_head != buffer_tail);
}

unsigned int buffer_peek(){
    return buffer[buffer_head];
}

unsigned int buffer_remove(){
    unsigned value = buffer[buffer_head];
    buffer_head = (buffer_head + 1) % RING_BUFFER_SIZE;
    return value;
}

void process_clock_interrupt(unsigned int pc){
    
    int current_bit = gpio_pin_read(GPIO_PIN24);  
   
    if(current_bit_index > 0 && current_bit_index < 9){
        current_scan_code = current_scan_code + (current_bit << (current_bit_index - 1));
    }

    if(current_bit_index == 10){
        buffer_insert(current_scan_code);
        current_bit_index = 0;
        current_scan_code = 0;
        gpio_check_and_clear_event(GPIO_PIN23);
        return;
    } 
     
    current_bit_index++;
    
    gpio_check_and_clear_event(GPIO_PIN23);

}

/* Initialize the keyboard driver.
   This should set up the keyboard GPIO pins and enable GPIO interrupts
   for them. It should not enable global interrupts: leave that to
   your main program. */
void keyboard_init() {
 


  int i;
  for(i = 0; i < RING_BUFFER_SIZE; i++){
    buffer[i] = 0;
  }
  
  PUT32(INTERRUPT_DISABLE_1, 0xffffffff);
  PUT32(INTERRUPT_DISABLE_2, 0xffffffff);

  // Put code to configure which GPIO events to trigger interrupts below

  
  // Bit 52 in IRQ registers enables/disables all GPIO interrupts
  // Bit 52 is in the second register, so subtract 32 for index
  PUT32(INTERRUPT_ENABLE_2, (1 << (52 - 32))); 

  // set up interrupt for clock pin
  gpio_set_input(GPIO_PIN23);
  gpio_set_pullup(GPIO_PIN23);
  gpio_detect_falling_edge(GPIO_PIN23);

  // interrupt not needed for data pin
  // ... but set-up as pull-up input
  gpio_set_input(GPIO_PIN24);
  gpio_set_pullup(GPIO_PIN24);


}



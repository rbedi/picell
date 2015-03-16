/* Rishi Bedi
 * CS107E
 * Assignment 4 - Extended
 */

#include "db.h"
#include "uart.h"
#include "console.h"

int GETR0();
int GETR3();
void SAVEREG(int* registers);

#define MAX_COMMAND_LENGTH 50


/* char* buf: buffer containing hexdump instruction
 * prints out 16 bytes starting from the memory address given in buf
 */
void hexdump(char* buf){
    int i;
    unsigned int memStartLoc = 0;
    int bufPos = 2; // skip over the 'x' and the ' '
    unsigned int base = 1;
    while(buf[bufPos]){
        bufPos++;
    }
    bufPos--;

    while(buf[bufPos] != 32 && buf[bufPos] != 'x'){  
        // count back until the end of the addr
        if(buf[bufPos] >= 65){  
            // letter (A-F)
            memStartLoc = memStartLoc + base * (buf[bufPos] - 'A' + 10); 
        }
        else if(buf[bufPos] <= 57){ 
            // number (0-9)
            memStartLoc = memStartLoc + base * (buf[bufPos] - '0');
        }
        bufPos--;
        base *= 16;
    }
    console_print("\n");
    for(i=0; i<16; i++){
        char * to_print = console_printf(" %2x", *(((char*)memStartLoc)+i)); 
        console_print(to_print);
    }
    return;
}

/*
 * int* registers: 16 int array of register values set to
 * what they were just before db was called
 * traces back, printing FP and LR for each stack frame
 */
void backtrace(int* registers){
    console_print("\n");
    int frame = 0;
    int* fp = (int*)registers[11]; // register 11 contains FP
    int* newFp = (int*)(*(fp-1));  // subtract 1 to get a pointer to the next FP
    char * to_print = console_printf("Frame %d: \nFP:0x%x\tLR:0x%x\n", frame, (int)fp, (int)(*fp));
    console_print(to_print);
    while((*newFp) != 0){ 
        // stop backtracing when LR is 0x0
        fp = newFp;
        newFp = (int*)(*(fp-1));    
        frame++;
        char* to_print = printf("Frame %d: \nFP:0x%x\tLR:0x%x\n", frame, (int)fp, (int)(*fp));
        console_print(to_print);
    }
}

/* int* registers: 16 int array of register values set to
 * what they were just before db was called
 * char* buf: buffer containing input command
 * main controller that dispatches the different command handlers
 */
bool process_input(int* registers, char* buf){
    int i;
    switch(buf[0]){
        case 'c':
            return false;
        case 'b':
            backtrace(registers);
            return true;
        case 'i':
            console_print("\n");
            for(i=0; i<16; i++){
                char* to_print = console_printf("r%d\t0x%2x\n", i, registers[i]);                
                console_print(to_print);
            } 
            return true;
        case 'x': 
            hexdump(buf);
            return true;
        default:
            console_print("\nNot a valid command");
            return true;
    }
}

/*
 * Main debugger function
 * Handles register saving and user input
 */
void db(void){
    int r0 = GETR0();
    int r3 = GETR3();
    int registers[16];
    SAVEREG(registers);
    registers[0] = r0;
    registers[3] = r3;
    registers[11] = (*(((int*)(registers[11]))-1));  // go to next FP so that r11 holds fp of caller
    registers[14] = *((int*)registers[11]); // set LR to LR of caller using caller FP
    while(1){
        console_print("\ndb>");
        char buf[MAX_COMMAND_LENGTH]; 
        buf[0] = '\0';
        int bufPos = 0;
        while(1){
            char ch = uart_getc();
            uart_putc(ch);
            if(ch == 13 || ch == 10){ // return - process the command
                buf[bufPos] = '\0';
                if(!process_input(registers, buf)){
                    printf("\n");
                    return;
                }
                else{
                    break;
                }
            }
            else if(ch == 8 || ch == 127){  
                // backspace
                if(bufPos > 0){
                    uart_putc(8); // backspace
                    uart_putc(32); // print a space
                    uart_putc(8); // backspace again
                    bufPos--;
                }
            }
            else{
                buf[bufPos] = ch;
                bufPos++;
            }
        }
    }
}

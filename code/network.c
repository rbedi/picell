#include "network.h"

void write(char reg, char data){
    reg |= 0x80;    // adds 1 to MSB to indicate write mode

    unsigned char out[2] = {reg, data};
    unsigned char in[2];
    transfer_bytes(out, in, 2, 0);

}

unsigned char read(char reg){
    reg &= 0x7F;
    unsigned char out[2] = {reg, 0xFF};
    unsigned char in[2];
    transfer_bytes(out, in, 2, 0);
    return in[1];
}

void radio_init(void){
    
    write(0x05, 0x00);  // disable all interrupts
    write(0x07, 0x01);  // set radio in ready mode
    write(0x09, 0x7F);  // set capacitance to 12.5pF
    write(0x0A, 0b00011111);  // set clock output to 32.768 kHz
    
    write(0x0F, 0x70);  // turn general purpose ADC off
    write(0x10, 0x00);  // turn general purpose ADC off

    write(0x12, 0x00);  // turn temp sensor off
    write(0x12, 0x00);  // turn temp sensor off
     
    // I don't really know what the next block does but it seems harmless enough.
    write(0x1C, 0x81    );  // IF filter bandwidth -- also 0x1D
    write(0x1D, 0x40);  // AFC loop
    write(0x20, 0x78);      // clock recovery -- also 0xA1
    write(0x21, 0x01);      // clock recovery -- also 0x20
    write(0x22, 0x11);      // clock recovery -- also 0x4E
    write(0x23, 0x11);      // clock recovery -- also 0xA5
    write(0x24, 0x01);      // clock recovery timing -- also 0x00
    write(0x25, 0x13);      // clock recovery timing -- also 0x0A

    write(0x2C, 0x28);    // OOK counter value MSBs
    write(0x2D, 0x0C);      // OOK counter value LSBs
    write(0x2E, 0x28);      
    write(0x1F, 0x03);      // ** writing to reserved? clocl recovery gearshift override

    write(0x69, 0x60);      // AGC override 1

    write(0x6E, 0x19);      // TX data rate 1
    write(0x6F, 0x9A);      // TX data rate 0

    write(0x30, 0x00);      // turning off packet handling

    // NEED TO SET PREAMBLE DETECTION CONTROL EVEN FOR DIRECT MODULATION
    write(0x33, 0b10000000);    // set Skipsyn to on (to skip sync word), no header
    write(0x34, 0x00);      // preamble length = 0 (same as 1 nibble = 4 bits)
    write(0x35, 0b00001010);    // set preamble detection threshold to 1 nibble (min), set RSSI offset to default +8 dB

    write(0x58, 0xC0);      // ?? also reserved

    write(0x6D, 0x07);      // maximize Tx power

    write(0x79, 0x00);      // no frequency hopping
    write(0x7A, 0x00);      // no frequency hopping
    
    // Modulation control
    write(0x70, 0x0C);    // Data rate below 30 kbps. Manchester, data whitening off
    write(0x71, 0x91);    // Tx Data CLK via SDO, Modulation via SDI, no inversion, OOK
    // 0x72 is for frequency deviation -- seems only relevant for FSK -- skipping?
    // Same for 0x73, 0x74 - frequency offset
    write(0x75, 0x53);      // 434 Mhz
    write(0x76, 0x64);      // 434 Mhz
    write(0x77, 0x00);      // 434 Mhz


}

void transmit(void){
    write(0x07, 0x09);      // to TX mode

    timer_wait_for(10000);

    char ch = read(0x02);

    gpio_set_function(GPIO_PIN10, GPIO_FUNC_OUTPUT);
    //gpio_set_function(GPIO_PIN9, GPIO_FUNC_INPUT);
    //gpio_set_pullup(GPIO_PIN9);
    //gpio_set_function(GPIO_PIN8, GPIO_FUNC_OUTPUT);

    timer_wait_for(10000);
    
    unsigned start_time = timer_gettime();

    while(1){

        if(timer_gettime() - start_time > 5000000){
         //   break;
        }

        while(!gpio_pin_read(GPIO_PIN9)){
            // do nothing while low
        }
        gpio_pin_write(GPIO_PIN10, 0);

        while(gpio_pin_read(GPIO_PIN9)){
            // do nothing while high
        }

        while(!gpio_pin_read(GPIO_PIN9)){
            // do nothing while low
        }
        gpio_pin_write(GPIO_PIN10, 1);
        
        while(gpio_pin_read(GPIO_PIN9)){
            // do nothing while high
        }
    } 

    spi_init(0,0);
    
    timer_wait_for(10000);

    write(0x07, 0x01);      // to ready mode
}

void receive(void){
   
    write(0x07, 0x05);      // to RX mode

    timer_wait_for(10000);

    gpio_set_function(GPIO_PIN10, GPIO_FUNC_INPUT);
//    gpio_set_function(GPIO_PIN9, GPIO_FUNC_INPUT);

    while(1){
        
        while(!gpio_pin_read(GPIO_PIN9)){
            // do nothing while low
        }     
        timer_wait_for(5);
        uart_putc('a' + gpio_pin_read(GPIO_PIN10));
        uart_putc('\n');
        
        while(gpio_pin_read(GPIO_PIN9)){
            // do nothing while high
        }

    }

}

void check_rssi(void){
    write(0x07, 0x05);
    timer_wait_for(10000);

    unsigned char ch = read(0x02);
    while(1){
        uart_putc('0' + ch);
        uart_putc('\n');
    }

}

void notmain(void){
    gpio_init();
    timer_init();
    uart_init();

    // CPOL = 0, meaning base value of the clock is 0
    // CPHA = 0, meaning data captured on rising edge (and propagated on falling edge)
    spi_init(0, 0);

    timer_wait_for(2000000);
    
    radio_init();

    timer_wait_for(1000);

     receive();
 //   transmit();
 //   check_rssi();

    unsigned char ch = read(0x71);

    while(1){
        uart_putc(ch - 0x40);
        uart_putc('\n');
        timer_wait_for(500000);
    } 
    return;
}

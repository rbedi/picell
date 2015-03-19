#include "network.h"

#define TRANSMIT 1 

// reg: 7-bit register to write into
// data: byte of data to write
void write(char reg, char data){
    reg |= 0x80;    // adds 1 to MSB to indicate write mode
    unsigned char out[2] = {reg, data};
    unsigned char in[2];
    transfer_bytes(out, in, 2, 0);

}

// reg: 7-bit register to read from
// Returns data stored in register. 
unsigned char read(char reg){
    reg &= 0x7F;    // ensures MSB is 0 to indicate read mode
    unsigned char out[2] = {reg, 0xFF};
    unsigned char in[2];
    transfer_bytes(out, in, 2, 0);
    return in[1];
}

// RFM 22B radio initialization.
void radio_init(void){
    
    write(0x05, 0x00);      // disable all interrupts
    write(0x07, 0x01);      // set radio in ready mode
    write(0x09, 0x7F);      // set capacitance to 12.5pF
    write(0x0A, 0x1F);      // set clock output to 32.768 kHz
    
    write(0x0F, 0x70);      // turn general purpose ADC off
    write(0x10, 0x00);      // turn general purpose ADC off

    write(0x12, 0x00);      // turn temp sensor off
    write(0x12, 0x00);      // turn temp sensor off
     
    write(0x1C, 0xB1);      // IF filter bandwidth -- also 0x1D
    write(0x1D, 0x40);      // AFC loop
    write(0x20, 0xBC);      // clock recovery 
    write(0x21, 0x00);      // clock recovery
    write(0x22, 0xAE);      // clock recovery
    write(0x23, 0xC3);      // clock recovery
    write(0x24, 0x00);      // clock recovery timing
    write(0x25, 0xB0);      // clock recovery timing

    write(0x2C, 0x28);      // OOK counter value MSBs
    write(0x2D, 0x9C);      // OOK counter value LSBs
    write(0x2E, 0x29);      
    write(0x1F, 0x03);      // clock recovery gearshift override

    write(0x69, 0x60);      // AGC override 1

    write(0x6E, 0x41);      // TX data rate 1
    write(0x6F, 0x89);      // TX data rate 0

    write(0x30, 0x00);      // turning off packet handling

    // Need to set preamble detection control, even for direct modulation.
    write(0x33, 0x80);      // set Skipsyn to on (to skip sync word), no header
    write(0x34, 0x00);      // preamble length = 0 (same as 1 nibble = 4 bits)
    write(0x35, 0x0A);      // set preamble detection threshold to 1 nibble (min), set RSSI offset to default +8 dB

    write(0x58, 0x80);      // Reserved register

    write(0x6D, 0x07);      // maximize Tx power

    write(0x79, 0x00);      // no frequency hopping
    write(0x7A, 0x00);      // no frequency hopping
    
    // Modulation control
    write(0x70, 0x2C);      // Data rate below 30 kbps. Manchester, data whitening off
    write(0x71, 0x91);      // Tx Data CLK via SDO, Modulation via SDI, no inversion, OOK
    
    // Set Tx/Rx freq
    write(0x75, 0x53);      // 434 Mhz
    write(0x76, 0x64);      // 434 Mhz
    write(0x77, 0x00);      // 434 Mhz


}

void transmit_preamble(unsigned repeat){
    int i;
    for(i = 0; i < repeat; i++){
        send_bit(0);
        send_bit(1);
    }
}

void send_bit(int bit){
    while(!gpio_pin_read(GPIO_PIN9)){
        // do nothing while low
    }
    gpio_pin_write(GPIO_PIN10, bit);

    while(gpio_pin_read(GPIO_PIN9)){
        // do nothing while high
    }
}

void transmit(char to_send){
    
    write(0x07, 0x09);      // to TX mode
    timer_wait_for(10000);  // wait 10 ms

    char ch = read(0x02);   // read and print current status byte
    printf("Current status: %d\n", (int)ch);

    // Set pin 10 as data modulation line.
    // Pin 9 being used as radio-defined clock line.
    gpio_set_function(GPIO_PIN10, GPIO_FUNC_OUTPUT);
    timer_wait_for(10000);
    
    // Need to transmit a preamble of 1s and 0s to make the RFM22B start receiving.
    // Need to contact manufacturer (HopeRF) in China to find a way to disable this,
    // as per the manual. Contacted them by phone and email, have not heard back yet. 
    transmit_preamble(20);

    int i;
    for(i = 0; i < 8; i++){
        int bit_to_send = to_send & 0x1; 
        if(bit_to_send){
            send_bit(0);
            send_bit(1);
            send_bit(1);
        }
        else{
            send_bit(1);
            send_bit(0);
            send_bit(0);
        }
        to_send = to_send >> 1;       
    }

    spi_init(0,0);          // Re-initialize
    timer_wait_for(10000);  // Wait for re-initialization for 10 ms
    write(0x07, 0x01);      // to ready mode

}

void receive(void){

    write(0x07, 0x05);      // to RX mode
    timer_wait_for(10000);  // wait for 10 ms

    // set pin 10 to input
    gpio_set_function(GPIO_PIN10, GPIO_FUNC_INPUT);
    
    // record start time of transmission
    unsigned cur_time = timer_gettime();

    while(1){
        while(!gpio_pin_read(GPIO_PIN9)){
            // do nothing while low
        }     
        unsigned int cur_bit = gpio_pin_read(GPIO_PIN10);
        uart_putc('0' + cur_bit);
        while(gpio_pin_read(GPIO_PIN9)){
            // do nothing while high
        }
    }

    // record end time of transmission
    cur_time = timer_gettime();

}

void print_rssi(void){
    int RSSI_val = check_rssi();
    printf("Current RSSI Reading: %d\n", RSSI_val);
}

int check_rssi(void){
    write(0x07, 0x05);
    timer_wait_for(10000);
    
    unsigned char ch = read(0x02);
    int RSSI_val = (int)ch;
    
    return RSSI_val;
}

void transmit_test(){
    while(1){
        transmit(0xFF);
    }
}

void notmain(void){
    gpio_init();
    timer_init();
    uart_init();

    // CPOL = 0, meaning base value of the clock is 0
    // CPHA = 0, meaning data captured on rising edge (and propagated on falling edge)
    spi_init(0, 0);

    // Pause for 1 second
    timer_wait_for(1000000);
    
    radio_init();

    // Pause for 1 millisecond
    timer_wait_for(1000);

    if(TRANSMIT){
        transmit_test();
    }
    else{ 
        receive();
    }

    return;
}

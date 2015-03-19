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
    write(0x33, 0b10000000);    // set Skipsyn to on (to skip sync word), no header
    write(0x34, 0x00);          // preamble length = 0 (same as 1 nibble = 4 bits)
    write(0x35, 0b00001010);    // set preamble detection threshold to 1 nibble (min), set RSSI offset to default +8 dB

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
    
    transmit_preamble(20);


    while(1){

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
    //gpio_set_function(GPIO_PIN26, GPIO_FUNC_INPUT);

    //gpio_set_function(GPIO_PIN9, GPIO_FUNC_INPUT);
    unsigned nsel = gpio_pin_read(GPIO_PIN8);


    int bits_received = 0;
    int counter[2];

    int i;

    unsigned cur_time = timer_gettime();


    while(timer_gettime() - cur_time < 2000000){
   // for(i=1; i < 10000; i++){
         
        while(!gpio_pin_read(GPIO_PIN26)){
            // do nothing while low
        }     
        
        unsigned int cur_bit = gpio_pin_read(GPIO_PIN10);


        
        while(gpio_pin_read(GPIO_PIN26)){
            // do nothing while high
        }
    }


    cur_time = timer_gettime();


    while(timer_gettime() - cur_time < 2000000){
   // for(i=1; i < 10000; i++){
         
        while(!gpio_pin_read(GPIO_PIN26)){
            // do nothing while low
        }     
        
        unsigned int cur_bit = gpio_pin_read(GPIO_PIN10);
        //uart_putc('a' + cur_bit);
        //uart_putc('\n');

        counter[cur_bit] += 1;

        // uart_putc('a' + gpio_pin_read(GPIO_PIN10));
        // uart_putc('\n');
        
        while(gpio_pin_read(GPIO_PIN26)){
            // do nothing while high
        }
    }
    while(1){
        printf("%d %d\n", counter[0], counter[1]);
        timer_wait_for(1000000);
    }
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

    receive();
 //    transmit();
 //   check_rssi();


    return;
}

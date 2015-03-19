#include "system.h"
#include "led.h"
#include "timer.h"
#include "gpio.h"
#include "uart.h"
#include "printf.h"
#include "utils.h"
#include "spi.h"

void write(char reg, char data);
unsigned char read(char reg);
void radio_init(void);
void transmit_preamble(unsigned repeat);
void send_bit(int bit);
void transmit(char to_send);
void receive(void);
void print_rssi(void);
int check_rssi(void);

void notmain(void);

/**
 * From Harry Johnson
 *
 */

#include "spi.h"
#include "gpio.h"
#include "timer.h"
#include "system.h"
#include "uart.h"

#define SPI_BASE 0x20204000
#define APB_CLK 250000000
#define FIFO_LEN 4
//FIFO 4 bytes long.

/*All of these are ALT0 of this pin*/
#define SPI0_CE1_N 7
#define SPI0_CE0_N 8
#define SPI0_MISO 9
#define SPI0_MOSI 10
#define SPI0_SCLK 11

typedef volatile struct spi_config_cs {
    /*  SPI0 CS Register        */
    unsigned int CS:2;
    unsigned int CPHA:1;
    unsigned int CPOL:1;
    unsigned int CLEAR:2;
    unsigned int CSPOL:1;
    unsigned int TA:1;
    unsigned int DMAEN:1;
    unsigned int INTD:1;
    unsigned int INTR:1;
    unsigned int ADCS:1;
    unsigned int REN:1;
    unsigned int LEN:1; //called LEN in the original documentation.
    unsigned int LMONO:1;
    unsigned int TE_EN:1;
    unsigned int DONE:1;
    unsigned int RXD:1;
    unsigned int TXD:1;
    unsigned int RXR:1;
    unsigned int RXF:1;
    unsigned int CSPOL0:1;
    unsigned int CSPOL1:1;
    unsigned int CSPOL2:1;
    unsigned int DMA_LEN:1;
    unsigned int LEN_LONG:1;
    unsigned int RESERVED:6;
}spi_config_cs;

    /*  SPI0 CLK Register     */
typedef volatile struct spi_config_clk {
    unsigned int CDIV:16;
    unsigned int RESERVED:16;
}spi_config_clk;

    /*  SPI0 DLEN Register     */
typedef volatile struct spi_config_dlen {
    unsigned int LEN:16;
    unsigned int RESERVED:16;
}spi_config_dlen;


    /*  SPI0 LTOH Register     */
typedef volatile struct spi_config_ltoh {
    unsigned int TOH:4;
    unsigned int RESERVED:28;
}spi_config_ltoh;

/*  SPI0 DC Register     */
typedef volatile struct spi_config_dc {
    unsigned int TDREQ:8;
    unsigned int TPANIC:8;
    unsigned int RDREQ:8;
    unsigned int RPANIC:8;
}spi_config_dc;

typedef volatile struct spi_config{
    spi_config_cs cs;
    unsigned int fifo;
    spi_config_clk clk;
    spi_config_dlen dlen;
    spi_config_ltoh ltoh;
    spi_config_dc dc;
}spi_config;


static volatile spi_config *spi_config_t = (spi_config*)(SPI_BASE);

void spi_gpio_init() {
    gpio_set_function(SPI0_CE0_N, GPIO_FUNC_ALT0);
    gpio_set_function(SPI0_CE1_N, GPIO_FUNC_ALT0);
    gpio_set_function(SPI0_MOSI, GPIO_FUNC_ALT0);
    gpio_set_function(SPI0_MISO, GPIO_FUNC_ALT0);
    gpio_set_function(SPI0_SCLK, GPIO_FUNC_ALT0);
}

int spi_init(volatile unsigned int CPOL, volatile unsigned int CPHA) {
    system_memory_write_barrier();
    spi_gpio_init();

    if(CPOL > CLOCK_REST_HIGH) return ERR;
    if(CPHA > SCLK_MID) return ERR;
    
    /* SPI CLK register.*/
    spi_config_t->clk.RESERVED = 0;
    spi_config_t->clk.CDIV = 4096; //End Frequency should be: 250000000/4096 = 61.035Khz.
    
    /* SPI DLEN register.*/
    spi_config_t->dlen.RESERVED = 0;
    spi_config_t->dlen.LEN = 0; //Number of bytes to transfer, only applies for DMA mode.
    
    /* SPI LTOH*/
    spi_config_t->ltoh.RESERVED = 0;
    spi_config_t->ltoh.TOH = 0; //1 clock hold time.
    
    /* SPI DC*/
    spi_config_t->dc.RPANIC = 0x30; /*Read Panic Threshold: increases priority of DREQ to RX DMA engine.*/
    spi_config_t->dc.RDREQ = 0x20; /*Read Request Threshold: DREQ to RX DMA engine when FIFO has this much data.*/
    spi_config_t->dc.TPANIC = 0x10;/*Same as above, but TX DMA*/
    spi_config_t->dc.TDREQ = 0x20; /*Same as above, but TX DMA*/
    
    /* SPI Chip Select Register.*/
    spi_config_t->cs.RESERVED = 0;
    spi_config_t->cs.LEN_LONG = 0; /* No Long data word for DMA_LEN + LOSSI */
    spi_config_t->cs.DMA_LEN = 0; /* No DMA in Lossi */
    spi_config_t->cs.CSPOL2 = 0; /* All chip select polarities active low. */
    spi_config_t->cs.CSPOL1 = 0;
    spi_config_t->cs.CSPOL0 = 0;
    spi_config_t->cs.TE_EN = 0; /* "Unused." Err..*/
    spi_config_t->cs.LMONO = 0; /* "Unused." Err..*/
    spi_config_t->cs.LEN = 0; /*Use SPI Master, not LoSSI Mode*/
    spi_config_t->cs.REN = 0; /*Read Enable for Bidirectional mode disabled.*/
    spi_config_t->cs.ADCS = 0; /*Don't automatically deassert chip select after DMA*/
    spi_config_t->cs.INTR = 0; /*Don't generate interrupt on RX FIFO */
    spi_config_t->cs.INTD = 0; /*Don't generate interrupt on transfer complete. */
    spi_config_t->cs.DMAEN = 0; /* DMA disabled. */
    spi_config_t->cs.TA = 0; /*Transfer not active.*/
    spi_config_t->cs.CSPOL = 0; /*All chip select lines active low.*/
    spi_config_t->cs.CLEAR = 0x3; /*Clear both TX and RX FIFO */
    spi_config_t->cs.CPOL = CPOL;
    spi_config_t->cs.CPHA = CPHA;
    spi_config_t->cs.CS = 0;
    system_memory_read_barrier();
    return INIT_OK;
}

int spi_transfer(unsigned char c) {
    unsigned int toReturn;
    while(!(spi_config_t->cs.TXD)); //wait for space in FIFO
    spi_config_t->fifo = c; //transmit char.
    while(!(spi_config_t->cs.DONE)); //wait until transaction done.
    while(!(spi_config_t->cs.RXD)); //wait for value in RX FIFO
    toReturn = (spi_config_t->fifo)&0xFF; //store value.
    return toReturn;
}

int transfer_bytes(unsigned char* out, unsigned char* in, unsigned int len, unsigned int CS) {
    if(CS > CS1) return ERR;
    if(CS > CS1) return ERR;
    spi_config_t->cs.CS = CS;
    spi_config_t->cs.CLEAR = 0x3; //clear FIFOs
    spi_config_t->cs.TA = 1; //transaction active, will set chip select to active mode.
    unsigned int ii;
    for(ii = 0; ii < len; ii++) {
        in[ii] = spi_transfer(out[ii]) & 0xFF;
    }
    spi_config_t->cs.TA = 0; //transaction not active, will set chip select to inactive mode.
    spi_config_t->cs.CLEAR = 0x0; //disable clear (not sure if this is necessary or not).
    system_memory_read_barrier();
    return 0;
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
 *
 * Author: Harry Johnson <harrymj@stanford.edu>
 * Date: 9/26/2014
 */

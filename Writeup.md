## Summary

PiCell is a library for using the RFM22B radio transceiver to send and receive messages. While the original goal was a bit more ambitious (involving implementing CDMA on top of this radio stack), the more basic goal of communicating between RFM22Bs was achieved. I used On-Off-Keying, and the "Direct Mode" of the radios, which presented one of the biggest (unexpected) challenges of the project -- getting these radios, which are generally intended for FIFO use where software on the radios handles packets for you, to allow me to directly stream bits over to be sent without additional shenanigans. The library as it stands right now allows you to send and receive bytes with a single library call -- it uses a slightly less efficient variant of Manchester Encoding (1 = 110, 0 = 001) to encode bits before sending them, necessary for systems that use On-Off Keying. 

## Challenges
* Antenna hardware
    
    Getting the store-bought antennas to work was tricky - finding the right adapter and then connecting them in such a way to get both the data modulation pin and the ground plate touching the right things on the radio. Ended up going with a jankier, but ultimately more reliable set-up of simply a 17cm piece of wire soldered to the antenna output pin. Was able to get reliable transmissions up to approximately 100 feet away, signal started significantly degrading after that. 

* Correct configuration values for 128 registers

    The radios have many useful features if they were being used independently of anything else. Unfortunately, these features were useless to me because I wanted to modulate which bits were being sent directly, so that I could encode messages using CDMA and the decode them on the receiver to identify the sender. This process would be significantly more complicated if there were additional bits being tacked on by the radio module that I did not have direct control over. Regardless, I needed to figure out what the various control registers for this radio did so that I could set only the ones I needed, while ensuring the rest did not do extra things that muck with my CDMA transmission. 

* Viewing the radio signal
    
    Pat (and Omar) lended me software defined radios (SDRs) to use to verify that I was sending a signal (and hopefully be able to view it as well). They were neat tools, but unfortunately the software I used (gqrx) was unable to show the the transmission at the time-resolution I needed to be able to distinguish between rapidly-transmitting zeroes and ones. They were, however, very helpful in knowing when I was transmitting anything at all, a good indicator that my transmit code was doing at least _something_. I then tried using the logic analyzer (which had been helpful in detecting the success / failure of my SPI transmissions) to view the radio signal, only to learn that its sample rate is orders of magnitude too low to detect a 434Mhz signal. Onto a oscilloscope at VAIL -- this, while closer to what I needed, was still not high-enough frequency to show anything but noise in a 434 MHz signal. What I ultimately needed was a spectrum analyzer to be able to reliably (and at high enough resolution) view exactly what was happening to the radio signal as I modulated it.  

* Using the Pi as a SPI slave

    Another nifty(?) thing about these radios is that they insist on providing the clock line for Direct Mode SPI transmissions. Meaning, they refuse to use the CLK master-provided line to define the falling edges on which data bits are sent/received. The RFM22B is happy to use this CLK line when its internal FIFO is used to manage when to send/receive bytes, but in Direct Mode, where I can directly control which bits are being sent without delay, the RFM22B defines its own clock line on SDO (MISO on the Pi) and uses SDI as the data line ... for both transmit and receive. This is annoying because the Pi is only configurable as a SPI master, as far as I could tell, not a SPI slave. This would be okay, still, since the SPI pins can simply be reconfigured as general purpose GPIO (and then put back to SPI after Tx/Rx is complete)... but the RFM22B simply fails to actually provide the clock line in Rx Direct Mode. It works perfectly fine in *Tx* Direct Mode (clock on MISO, data on MOSI), but in Rx Direct Mode, _only_ that Data line is provided. The strange thing is that the data bits provided back on the Data line are correct and timed correctly -- meaning there is clearly an internal clock in the radio chip that is timing received bits correctly -- it just fails to provide that clock line on the SDO pin as promised (and as Tx mode correctly does). My hack around this was to, at the beginning of the Rx process, listen to the preamble and time the bit transitions on a "101010..." transmission to figure out how long each transition took, and then using this value as a sort of de-facto clock. This does not work very reliably, since the call to the system clock on the Pi takes about as long as the period of one transmission (~5-10us), but provided at least some way of getting around this issue. 

## Cool things I learned in the process
* How to use a logic analyzer! Such a neat tool.
* (With help from Harry), how to use an oscilloscope
* FSK, OOK, ASK -- all the different ways radios transmit things, and all of the things radios are capable of doing in hardware and software to improve their transmissions.
* While I did not end up implementing CDMA, I learned a lot about it - and implemented a wired CDMA simulator (concerned that the radio would not work at all...).
* SPI (using a library Harry wrote) 
* How to debug a hardware/software setup with so many parts that could be independently failing.

## Acknowledgements
* Phil & Pat for lots of help with various parts of the project, and the use of much hardware!
* Harry, for use of the SPI library and helping me try to figure things out with the oscilloscope last minute
* Omar, for the use of his SDR

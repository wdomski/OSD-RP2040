# OSD-RP2040
RP2040 based On-Screen Display with I2C interface

This example project shows implementation of OSD (on screen device) 
using RP2040 microcontroller, a piece which could be found on 
Raspberry Pi Pico boards.

Details about the implementationcan can be found on my blog 
[On-screen Display with Raspberry Pi Pico](https://blog.domski.pl/on-screen-display-with-raspberry-pi-pico/).

## Technical detials
The OSD was implemented using SPI interface and MOSI line, hence it 
is easy to scale font width just by adjusting the SPI clock. 
Moreover, DMA is being used to output the lines on the screen.

The OSD puts four lines of text, two are at the top and the 
other two are at the bottom.

It utilizes the LM1881 chip which extract synchronization impluses of 
the vide signal. The hardware design of LM1881 based board can be 
found in [this repository](https://github.com/wdomski/OSD-KiCAD). 

### Device communication interface
In order to put some specific parameters on the screen an I2C 
slave device was implemented. Thanks to it it maps memory registers 
available via I2C bus directly to variables.


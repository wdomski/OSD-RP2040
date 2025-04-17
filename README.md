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

## Frame buffer support

Current version of OSD supports a single frame buffer defined as
```C
uint8_t frame_buffer[OSD_LINE_LENGTH * OSD_LINES];
```

To retrieve a specific line of the frame buffer, the following function can be used:
```C
uint8_t * get_line(uint8_t *buffer, uint32_t line_number)
```

## Font

Font is defined in `fonts.h` file. Each character is 8x8 while each line 
corresponds to a horizontal line of the screen. 

Frame buffer can be filled with text using following functions:

- display text in a given line
```C
void font_print_line(uint8_t *font, uint8_t *input, uint32_t input_length,
					 uint32_t line_length, uint32_t horizontal_char_offset, uint8_t *buffer);
```

- display text in a given line with scaling
```C
void font_print_line_scale(uint8_t *font, uint8_t *input, uint32_t input_length,
						   uint32_t line_length, uint32_t horizontal_char_offset, uint8_t *buffer,
						   uint8_t x_scale, uint8_t y_scale);
```

- put blan characters (spaces) in a given line
```C
void fill_with_blanks(uint8_t *buffer, int32_t length);
```

## Device communication interface
In order to put some specific parameters on the screen an I2C 
slave device was implemented. Thanks to it it maps memory registers 
available via I2C bus directly to variables.

I2C communication supports CRC calculation with CRC8, so the data integrity is
ensured. 

The data is set using a packed structure `osd_data_t` which is defined in `osd-comm.h` file.

```C
typedef struct{
    osd_data_length_t message_length;
    int16_t battery;
    int16_t rssi;
    int32_t gps_altitude;
    int32_t gps_velocity;
    int32_t gps_latitude;
    int32_t gps_longitude;
    uint8_t gps_sat_number;
    int32_t gps_distance;
    int32_t gps_distance_on_ground;
    int32_t gps_heading;
    int32_t nav_home_angle_deviation;
    uint32_t gps_time;
    uint8_t osd_enabled;
    int32_t ctrl_signal;
    int32_t vertical_speed;
    uint8_t autopilot_enabled;
    osd_param_config_t param_config;
    osd_data_crc_t crc;
} __packed osd_data_t;
```
/*
 * osd.c
 *
 *  Created on: Feb 28, 2025
 *      Author: Wojciech_Domski
 */

#include <stdint.h>

#include "osd.h"

uint8_t frame_buffer[OSD_LINE_LENGTH * OSD_LINES];

uint8_t * get_line(uint8_t *buffer, uint32_t line_number)
{
    return &buffer[OSD_LINE_LENGTH * line_number];
}

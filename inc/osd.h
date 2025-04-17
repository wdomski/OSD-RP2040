/*
 * osd.h
 *
 *  Created on: Feb 28, 2025
 *      Author: Wojciech_Domski
 */

#ifndef INC_OSD_H_
#define INC_OSD_H_

#include <stdint.h>

#define OSD_LINES 250
#define OSD_LINE_LENGTH 75

// number of lines (not characters!) to skip
#define FRAME_BUFFER_Y_OFFSET_LINES 11
// number of characters ! (not lines) to skip
// so for font 8x8 and value of offset 3 the horizontal line offset is 8x3 = 24
#define FRAME_BUFFER_X_OFFSET_CHARS 4

extern uint8_t frame_buffer[];

uint8_t * get_line(uint8_t *buffer, uint32_t line_number);

#endif /* INC_OSD_H_ */

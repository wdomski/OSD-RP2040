/*
 * fonts.h
 *
 *  Created on: Jul 28, 2022
 *      Author: Wojciech_Domski
 */

#ifndef INC_FONTS_H_
#define INC_FONTS_H_

#include <stdint.h>

extern uint8_t font_small[][8];

void font_print_line(uint8_t *font, uint8_t *input, uint32_t input_length,
		uint8_t *buffer);

#endif /* INC_FONTS_H_ */

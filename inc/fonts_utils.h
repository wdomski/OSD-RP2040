/*
 * fonts_utils.h
 *
 *  Created on: Feb 28, 2025
 *      Author: Wojciech_Domski
 */

#ifndef INC_FONTS_UTILS_H_
#define INC_FONTS_UTILS_H_

#include <stdint.h>

void font_print_line(uint8_t *font, uint8_t *input, uint32_t input_length, uint32_t line_length,
					 uint32_t horizontal_char_offset, uint8_t *buffer);

void font_print_line_scale(uint8_t *font, uint8_t *input, uint32_t input_length, uint32_t line_length,
						   uint32_t horizontal_char_offset, uint8_t *buffer, uint8_t x_scale, uint8_t y_scale);

void fill_with_blanks(uint8_t *buffer, int32_t length);

#endif /* INC_FONTS_UTILS_H_ */

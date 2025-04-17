/*
 * fonts_utils.c
 *
 *  Created on: Feb 28, 2025
 *      Author: Wojciech_Domski
 */

#include <stdint.h>

#include "fonts_utils.h"

void font_print_line(uint8_t *font, uint8_t *input, uint32_t input_length,
					 uint32_t line_length, uint32_t horizontal_char_offset, uint8_t *buffer)
{
	uint8_t sign;
	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < input_length; ++j)
		{
			sign = input[j];
			*(buffer + i * line_length + j + horizontal_char_offset) = (font + 8 * sign)[i];
		}
	}
}

void font_print_line_scale(uint8_t *font, uint8_t *input, uint32_t input_length,
						   uint32_t line_length, uint32_t horizontal_char_offset, uint8_t *buffer,
						   uint8_t x_scale, uint8_t y_scale)
{
	uint8_t sign[10]; // defines maximum x scale to 10

	if (x_scale > 10)
	{
		x_scale = 10;
	}

	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < input_length; ++j)
		{
			if (x_scale == 1)
			{
				sign[0] = input[j];
				for (int y = 0; y < y_scale; ++y)
				{
					*(buffer + (i * y_scale + y) * line_length + j + horizontal_char_offset) = (font + 8 * sign[0])[i];
				}
			}
			else
			{
				// clear scalled sign
				for (int x = 0; x < x_scale; ++x)
				{
					sign[x] = 0x00;
				}

				// scale horizontally by
				// duplicating given bit exactly x_scale times
				uint8_t character = (font + 8 * input[j])[i];
				for (int k = 0; k < 8; ++k)
				{
					if (character & (1 << k))
					{
						for (int x = 0; x < x_scale; ++x)
						{
							uint8_t offset = (x_scale * 8 - 1) - (k * x_scale + x);
							uint8_t byte_offset = offset / 8;
							uint8_t bit_offset = offset % 8;
							sign[byte_offset] |= 1 << (7 - bit_offset);
						}
					}
				}

				for (int x = 0; x < x_scale; ++x)
				{
					for (int y = 0; y < y_scale; ++y)
					{
						*(buffer + (i * y_scale + y) * line_length + horizontal_char_offset + j * x_scale + x) = sign[x];
					}
				}
			}
		}
	}
}

void fill_with_blanks(uint8_t *buffer, int32_t length)
{
    for (int i = 0; i < length; ++i)
    {
        buffer[i] = ' ';
    }
}
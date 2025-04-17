#ifndef I2C_SLAVE_HEADER
#define I2C_SLAVE_HEADER

#include "hardware/i2c.h"

#include <pico/i2c_slave.h>

extern volatile uint8_t osd_i2c_slave_buffer[];

/**
 * @brief Initialize hardware for I2C slave operation
 * 
 */
void i2c_slave_setup(void);

/**
 * @brief Handler for I2C events
 * 
 * @param i2c I2C instance
 * @param event I2C event
 */
void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event);

#endif

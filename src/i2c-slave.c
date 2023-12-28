#include <stdint.h>

#include "i2c-slave.h"
#include "hardware/gpio.h"

static const uint I2C_SLAVE_ADDRESS = 0x17;
static const uint I2C_SLAVE_BAUDRATE = 400000;
static const uint I2C_SLAVE_WHO_AM_I = 0xAA;

static const uint I2C_SLAVE_SDA_PIN = 4;
static const uint I2C_SLAVE_SCL_PIN = 5;
static i2c_inst_t *I2C_SLAVE_INST = i2c0;

uint8_t i2c_slave_cmd;
uint8_t i2c_slave_cmd_address;
uint8_t i2c_slave_cmd_written;

volatile uint8_t i2c_slave_buffer[64];

void i2c_slave_setup(void)
{
    gpio_init(I2C_SLAVE_SDA_PIN);
    gpio_set_function(I2C_SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SDA_PIN);

    gpio_init(I2C_SLAVE_SCL_PIN);
    gpio_set_function(I2C_SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SCL_PIN);

    i2c_init(I2C_SLAVE_INST, I2C_SLAVE_BAUDRATE);
    // configure I2C0 for slave mode
    i2c_slave_init(I2C_SLAVE_INST, I2C_SLAVE_ADDRESS, &i2c_slave_handler);
}

static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event)
{
    switch (event)
    {
    case I2C_SLAVE_RECEIVE: // master has written some data
    {
        if (!i2c_slave_cmd_written)
        {
            i2c_slave_cmd_address = i2c_read_byte_raw(I2C_SLAVE_INST);
            i2c_slave_cmd_written = 1;
        }
        else
        {
            i2c_slave_buffer[i2c_slave_cmd_address++] = i2c_read_byte_raw(I2C_SLAVE_INST);
        }
        break;
    }
    case I2C_SLAVE_REQUEST: // master is requesting data
    {
        // send dummy data
        i2c_write_byte_raw(i2c, I2C_SLAVE_WHO_AM_I);
        break;
    }
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
    {
        i2c_slave_cmd_written = 0;
        i2c_slave_cmd_address = 0;
        break;
    }
    default:
    {
        break;
    }
    }
}

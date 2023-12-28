#ifndef I2C_SLAVE_HEADER
#define I2C_SLAVE_HEADER

#include "hardware/i2c.h"

#include <pico/i2c_slave.h>

extern volatile uint8_t i2c_slave_buffer[];

#define  I2C_DATA_BUFFER i2c_slave_buffer

// in mV (mili volts)
#define OSD_REG_BATTERY  (*(int32_t*)I2C_DATA_BUFFER)

#define OSD_REG_RSSI  (*(int32_t*)(I2C_DATA_BUFFER+4))

// in mm (mili meters)
#define OSD_REG_GPS_ALTITUDE  (*(int32_t*)(I2C_DATA_BUFFER+8))

// in m/h (meters per hour)
#define OSD_REG_GPS_VELOCITY  (*(int32_t*)(I2C_DATA_BUFFER+12))

// in mdeg (mili degrees)
#define OSD_REG_GPS_LATITUDE  (*(int32_t*)(I2C_DATA_BUFFER+16))

// in mdeg (mili degrees)
#define OSD_REG_GPS_LONGITUDE  (*(int32_t*)(I2C_DATA_BUFFER+20))

// numer of satellites
#define OSD_REG_GPS_SAT_NUMBER  (*(int32_t*)(I2C_DATA_BUFFER+24))

// in mm (milimeters)
#define OSD_REG_GPS_DISTANCE  (*(int32_t*)(I2C_DATA_BUFFER+28))

// in mm (milimeters)
#define OSD_REG_GPS_DISTANCE_ON_GROUND  (*(int32_t*)(I2C_DATA_BUFFER+32))

// in deg
#define OSD_REG_GPS_HEADING (*(int32_t*)(I2C_DATA_BUFFER+36))

// in deg
#define OSD_REG_NAV_HOME_ANGLE_DEVIATION (*(int32_t*)(I2C_DATA_BUFFER+40))

// current time in HHMMSS format
#define OSD_REG_GPS_TIME  (*(int32_t*)(I2C_DATA_BUFFER+44))

// 0 or 1
#define OSD_REG_OSD_ENABLED  (*(int32_t*)(I2C_DATA_BUFFER+48))

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
static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event);

#endif

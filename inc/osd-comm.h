/*
* osd-comm.h
*
*  Created on: 03 Mar 2025
*      Author: Wojciech Domski
*/

#ifndef INC_OSD_COMM_H_
#define INC_OSD_COMM_H_

#include <stdint.h>

#if defined(__packed)
#else
#if defined(__MINGW32__)
#define __packed __attribute__((__gcc_struct__, __packed__, __aligned__(1)))
#else
#define __packed __attribute__((__packed__, __aligned__(1)))
#endif
#endif

typedef uint8_t osd_data_length_t;
typedef uint8_t osd_data_crc_t;
typedef struct
{
    uint8_t mode;  // 0 disabled, 1 param selection, 2 param value modification
    uint8_t index; // parameter index
    int32_t value; // parameter value
} osd_param_config_t;

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

extern volatile uint8_t osd_data_buffer[];

extern volatile osd_data_t * osd_data;

#define OSD_DEVICE_ADDRESS 0x17

//first block
// do not use, ONLY for buffer length calculation
#define OSD_REG_FIRST_BLOCK  osd_data_buffer

// length (in bytes) of all messages including CRC
#define OSD_REG_MESSAGE_LENGTH  osd_data->message_length

// in mV (mili volts)
#define OSD_REG_BATTERY  osd_data->battery

// RSSI
#define OSD_REG_RSSI  osd_data->rssi

// in mm (mili meters)
#define OSD_REG_GPS_ALTITUDE  osd_data->gps_altitude

// in m/h (meters per hour)
#define OSD_REG_GPS_VELOCITY  osd_data->gps_velocity

// in mdeg (mili degrees)
#define OSD_REG_GPS_LATITUDE  osd_data->gps_latitude

// in mdeg (mili degrees)
#define OSD_REG_GPS_LONGITUDE  osd_data->gps_longitude

// number of satellites
#define OSD_REG_GPS_SAT_NUMBER  osd_data->gps_sat_number

// in mm (milimeters)
#define OSD_REG_GPS_DISTANCE  osd_data->gps_distance

// in mm (milimeters)
#define OSD_REG_GPS_DISTANCE_ON_GROUND  osd_data->gps_distance_on_ground

// in deg
#define OSD_REG_GPS_HEADING  osd_data->gps_heading

// in deg
#define OSD_REG_NAV_HOME_ANGLE_DEVIATION  osd_data->nav_home_angle_deviation

// current time in HHMMSS format
#define OSD_REG_GPS_TIME  osd_data->gps_time

// 0 or 1
#define OSD_REG_OSD_ENABLED  osd_data->osd_enabled

// ctrl signal value for autopilot scaled up by 1000
#define OSD_REG_CTRL_SIGNAL  osd_data->ctrl_signal

// vertical speed in mm/s
#define OSD_REG_VERTICAL_SPEED  osd_data->vertical_speed

// autopilot mode, 0 - disabled, 1 - enabled
#define OSD_REG_AUTOPILOT_ENABLED  osd_data->autopilot_enabled

// parameter config number, 0 - no parameter, 1 - 255 parameter number
#define OSD_REG_PARAM_CONFIG_NUMBER  osd_data->param_config_number

// parameter config value
#define OSD_REG_PARAM_CONFIG_VALUE  osd_data->param_config_value

// CRC
#define OSD_REG_CRC  osd_data->crc

#define OSD_PAYLOAD_LENGTH  (sizeof(osd_data_t) - sizeof(osd_data_length_t) - sizeof(osd_data_crc_t))
#define OSD_CRC_LENGTH  sizeof(osd_data_crc_t)
#define OSD_MESSAGE_LENGTH  sizeof(osd_data_length_t)

// entire buffer length in bytes
#define OSD_BUFFER_LENGTH  sizeof(osd_data_t)

uint8_t osd_crc(const uint8_t *buffer, uint8_t length);

#endif /* INC_OSD_COMM_H_ */

/*
* osd-param.h
*
*  Created on: 06 Mar 2025
*      Author: Wojciech Domski
*/

#ifndef INC_OSD_PARAM_H_
#define INC_OSD_PARAM_H_

enum{
    OSD_PARAM_INDEX_PID_AUTO_PILOT_P = 0,
    OSD_PARAM_INDEX_PID_AUTO_PILOT_I,
    OSD_PARAM_INDEX_PID_AUTO_PILOT_D,
    OSD_PARAM_INDEX_PID_AUTO_PILOT_P_LIMIT,
    OSD_PARAM_INDEX_PID_AUTO_PILOT_I_LIMIT,
    OSD_PARAM_INDEX_PID_AUTO_PILOT_D_LIMIT,
    OSD_PARAM_INDEX_PID_AUTO_PILOT_TOTAL_LIMIT,
    OSD_PARAM_INDEX_CH_OUT_1_INV_AILERON_1,
    OSD_PARAM_INDEX_CH_OUT_2_INV_ELEVATOR,
    OSD_PARAM_INDEX_CH_OUT_3_INV_THROTTLE,
    OSD_PARAM_INDEX_CH_OUT_4_INV_RUDDER,
    OSD_PARAM_INDEX_CH_OUT_5_INV_THROTTLE_2,
    OSD_PARAM_INDEX_CH_OUT_6_INV_AILERON_2,
    OSD_PARAM_INDEX_CH_OUT_1_SCALE_AILERON_1,
    OSD_PARAM_INDEX_CH_OUT_2_SCALE_ELEVATOR,
    OSD_PARAM_INDEX_CH_OUT_3_SCALE_THROTTLE,
    OSD_PARAM_INDEX_CH_OUT_4_SCALE_RUDDER,
    OSD_PARAM_INDEX_CH_OUT_5_SCALE_THROTTLE_2,
    OSD_PARAM_INDEX_CH_OUT_6_SCALE_AILERON_2,
    OSD_PARAM_INDEX_COUNT
};

extern const char * osd_param_names[];

extern const unsigned int osd_param_fraction[];

extern const signed int osd_param_min_value[];

extern const signed int osd_param_max_value[];

#define OSD_PARAM_COUNT  OSD_PARAM_INDEX_COUNT

#endif /* INC_OSD_PARAM_H_ */

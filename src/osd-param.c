/*
 * osd-param.c
 *
 *  Created on: 06 Mar 2025
 *      Author: Wojciech Domski
 */

 #include "osd-param.h"

 const char *osd_param_names[] = {
     "PID AUTO PILOT P",
     "PID AUTO PILOT I",
     "PID AUTO PILOT D",
     "PID AUTO PILOT P LIMIT",
     "PID AUTO PILOT I LIMIT",
     "PID AUTO PILOT D LIMIT",
     "PID AUTO PILOT TOTAL LIMIT",
     "CH OUT 1 INV AILERON 1",
     "CH OUT 2 INV ELEVATOR",
     "CH OUT 3 INV THROTTLE",
     "CH OUT 4 INV RUDDER",
     "CH OUT 5 INV THROTTLE 2",
     "CH OUT 6 INV AILERON 2",
     "CH OUT 1 SCALE AILERON 1",
     "CH OUT 2 SCALE ELEVATOR",
     "CH OUT 3 SCALE THROTTLE",
     "CH OUT 4 SCALE RUDDER",
     "CH OUT 5 SCALE THROTTLE 2",
     "CH OUT 6 SCALE AILERON 2",
 };
 
 const unsigned int osd_param_fraction[] = {
     100, // PID AUTO PILOT P
     100,
     100,
     10,
     10,
     10,
     10,
     100, // CH OUT 1 INV AILERON 1
     100,
     100,
     100,
     100,
     100,
     100, // CH OUT 1 SCALE AILERON 1
     100,
     100,
     100,
     100,
     100,
 };
 
 const signed int osd_param_min_value[] = {
     0, // PID AUTO PILOT P
     0,
     0,
     0,
     0,
     0,
     0,
     -100, // CH OUT 1 INV AILERON 1
     -100,
     -100,
     -100,
     -100,
     -100,
     0, // CH OUT 1 SCALE AILERON 1
     0,
     0,
     0,
     0,
     0,
 };
 
 const signed int osd_param_max_value[] = {
     200, // PID AUTO PILOT P
     200,
     200,
     1000,
     1000,
     1000,
     1000,
     100, // CH OUT 1 INV AILERON 1
     100,
     100,
     100,
     100,
     100,
     200, // CH OUT 1 SCALE AILERON 1
     200,
     200,
     200,
     200,
     200,
 };
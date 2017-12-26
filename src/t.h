#ifndef _THERMOSTAT_T_H
#define _THERMOSTAT_T_H

#include <Arduino.h>
#include <OneWire.h>




extern float t[];

extern bool sensor_init();
extern void sensor_loop();
extern void sensor_shift();

#endif // _THERMOSTAT_T_H


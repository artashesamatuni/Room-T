#ifndef _THERMOSTAT_KEYPAD_H
#define _THERMOSTAT_KEYPAD_H


#define _KEY_DUMMY  0
#define _KEY_RIGHT  1
#define _KEY_LEFT   2
#define _KEY_OK     3


#include <Arduino.h>

extern uint8_t  tbtn;

void key_loop();

#endif // _THERMOSTAT_KEYPAD_H


#ifndef _THERMOSTAT_LCD_H
#define _THERMOSTAT_LCD_H

#define _M_HOME     0
#define _M_TIME     1
#define _M_MQTT     2
#define _M_TEMP     3
#define _M_WEB      4
#define _M_NET      5
#define _M_SYS      6
#define _M_STAT     7
#define _M_CLOCK    8

#define _DAY_WEEKDAY  0
#define _DAY_WEEKEND  1

#include <SPI.h>
#include "Ucglib.h"





void tft_init(void);
void tftConsole(String a, String b);
void tft_update(void);
void tft_loop(uint8_t scr);














#endif // _THERMOSTAT_LCD_H

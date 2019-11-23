#ifndef _ESP_WEATHER_H
#define _ESP_WEATHER_H

#include <Arduino.h>
            // Create class for HTTPS TCP connections get_https()


extern void weather_client_setup();
extern void weather_client_loop();
void makehttpRequest();
void parseJson(const char * jsonString);
void diffDataAction(String nowT, String later, String weatherType);





#endif // _ESP_WEATHER_H

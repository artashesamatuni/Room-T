#ifndef _THERMOSTAT_NTP_H
#define _THERMOSTAT_NTP_H

#include <Arduino.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include "RTClib.h"

extern RTC_Millis rtc;


boolean ntp_setup();
unsigned long sendNTPpacket(IPAddress& address);
void ntp_restart();

#endif // _THERMOSTAT_NTP_H


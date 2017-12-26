#ifndef _ESP_H
#define _ESP_H

// -------------------------------------------------------------------
// General support code used by all modules
// -------------------------------------------------------------------

// Uncomment to use hardware UART 1 for debug else use UART 0
//#define DEBUG_SERIAL1

#ifdef DEBUG_SERIAL1
#define DEBUG Serial1
#else
#define DEBUG Serial
#endif
//DS18B20
#define ds1820      2

//Keyboard
#define KEYBOARD    A0

//TFT
#define TFT_CS      15
#define TFT_RST     5
#define TFT_DC      4
#define TFT_SCLK    14
#define TFT_MOSI    13

#define HEATER     16
#define COOLER     0

#define WIFI_LED     2 //nodemcu 1.0 eLED


#endif // _ESP_H


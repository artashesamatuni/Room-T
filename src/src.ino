#include "esp.h"
#include "config.h"
#include "wifi.h"
#include "ntp.h"
#include "web_server.h"
#include "emoncms.h"
#include "mqtt.h"
#include "lcd.h"
#include "t.h"
#include "keypad.h"

unsigned long t_mqtt = 0;
unsigned long t_emon = 0;
// -------------------------------------------------------------------
// SETUP
// -------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
#ifdef DEBUG_SERIAL1
  Serial1.begin(115200);
#endif
  console("SERIAL", String(ESP.getChipId()));
  console("FIRMWARE", currentfirmware);
  config_load_settings();
  tft_init();
  console("Sensors", (sensor_init()) ? "OK" : "FAIL");
  wifi_setup();
  console("NTP", (ntp_setup()) ? "OK" : "FAIL");
  web_server_setup();
  tft_update();
  scr_pos = _M_HOME;
  menu_open = false;
}

// -------------------------------------------------------------------
// LOOP
// -------------------------------------------------------------------
void loop()
{
  key_loop();
  tft_loop(scr_pos);
  sensor_loop();
  web_server_loop();
  wifi_loop();

  String input = "";
  input = "Current-t:" + String(ct) + ",Target-t:" + String(sp);

  if (wifi_mode == WIFI_MODE_STA || wifi_mode == WIFI_MODE_AP_AND_STA)
  {
    if (emon_interval != 0 && emon_apikey != 0 && (millis() - t_emon) >= (emon_interval * 1000))
    {
      emon_publish(input);
      t_emon = millis();
    }
    if (mqtt_server != 0 && mqtt_interval != 0)
    {
      mqtt_loop();
      if ((millis() - t_mqtt) >= (mqtt_interval * 1000))
      {
        mqtt_publish(input);
        t_mqtt = millis();
      }
    }
  }
} // end loop



#ifndef _EMONESP_CONFIG_H
#define _EMONESP_CONFIG_H

#include <Arduino.h>

// -------------------------------------------------------------------
// Load and save the EmonESP config.
//
// This initial implementation saves the config to the EEPROM area of flash
// -------------------------------------------------------------------

// Global config varables

// Wifi Network Strings
extern String esid;
extern String epass;

// Web server authentication (leave blank for none)
extern String www_username;
extern String www_password;

// EMONCMS SERVER strings
extern String emon_server;
extern String emon_node;
extern String emon_apikey;
extern String emon_fingerprint;
extern uint16_t emon_interval;

// MQTT Settings
extern String mqtt_server;
extern String mqtt_topic;
extern String mqtt_user;
extern String mqtt_pass;
extern String mqtt_feed_prefix;
extern uint16_t mqtt_port;
extern uint16_t mqtt_interval;

//TIME
extern int8_t ntp_tz;
extern uint8_t ntp_ip[];

//Temperature
extern float    ct;
extern uint8_t  sp;


extern uint8_t keystatus;

//System
extern uint8_t scr_pos;
extern bool menu_open;
extern uint8_t menu_pos;

// Shedule Settings
extern uint8_t sha[];
extern uint16_t schedule_time[];
extern uint8_t b[];
extern bool  update_shed;
// -------------------------------------------------------------------
// Load saved settings
// -------------------------------------------------------------------
extern void config_load_settings();

void config_save_sp();
// -------------------------------------------------------------------
// Save the EmonCMS server details
// -------------------------------------------------------------------
extern void config_save_emon(String server, String node, String apikey, String fingerprint, String interval);

// -------------------------------------------------------------------
// Save the MQTT broker details
// -------------------------------------------------------------------
extern void config_save_mqtt(String server, String topic, String prefix, String user, String pass,String port, String interval);

// -------------------------------------------------------------------
// Save the admin/web interface details
// -------------------------------------------------------------------
extern void config_save_admin(String user, String pass);

// -------------------------------------------------------------------
// Save the NTP details
// -------------------------------------------------------------------
extern void config_save_ntp(String tz, String ip1, String ip2, String ip3, String ip4);

// -------------------------------------------------------------------
// Save the Wifi details
// -------------------------------------------------------------------
extern void config_save_wifi(String qsid, String qpass);


// -------------------------------------------------------------------
// Reset the config back to defaults
// -------------------------------------------------------------------
extern void config_reset();
void console(String a, String b);
#endif // _EMONESP_CONFIG_H


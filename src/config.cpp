#include "esp.h"
#include "config.h"

#include <Arduino.h>
#include <EEPROM.h>                   // Save config settings

// Wifi Network Strings
String esid = "";
String epass = "";

// Web server authentication (leave blank for none)
String www_username = "";
String www_password = "";

// EMONCMS SERVER strings
String emon_server = "";
String emon_node = "";
String emon_apikey = "";
String emon_fingerprint = "";
uint16_t emon_interval = 10;

// MQTT Settings
String mqtt_server = "";
String mqtt_topic = "";
String mqtt_user = "";
String mqtt_pass = "";
String mqtt_feed_prefix = "";
uint16_t mqtt_port = 1883;
uint16_t mqtt_interval = 10;

//TIME
//216.229.0.179  time.nist.gov NTP server
int8_t ntp_tz;
uint8_t ntp_ip[4] = {129, 6, 15, 28};

//Temperature
float    ot;
float    ct;
uint8_t  sp;
uint8_t keystatus;


//System
uint8_t scr_pos;
bool menu_open;
uint8_t menu_pos;

// Shedule Settings
uint8_t sha[24];
uint16_t schedule_time[24];
uint8_t b[24];



#define EEPROM_ESID_SIZE          32
#define EEPROM_EPASS_SIZE         64
#define EEPROM_EMON_API_KEY_SIZE  32
#define EEPROM_EMON_SERVER_SIZE   45
#define EEPROM_EMON_NODE_SIZE     32
#define EEPROM_EMON_INTERVAL_SIZE 2

#define EEPROM_MQTT_SERVER_SIZE   45
#define EEPROM_MQTT_TOPIC_SIZE    32
#define EEPROM_MQTT_USER_SIZE     32
#define EEPROM_MQTT_PASS_SIZE     64
#define EEPROM_MQTT_PORT_SIZE     2
#define EEPROM_MQTT_INTERVAL_SIZE 2

#define EEPROM_EMON_FINGERPRINT_SIZE  60
#define EEPROM_MQTT_FEED_PREFIX_SIZE  10
#define EEPROM_WWW_USER_SIZE      16
#define EEPROM_WWW_PASS_SIZE      16

#define EEPROM_NTP_TZ_SIZE   1
#define EEPROM_NTP_IP_SIZE  4
#define EEPROM_SP_SIZE  1

#define EEPROM_SIZE 512

#define EEPROM_ESID_START         0
#define EEPROM_ESID_END           (EEPROM_ESID_START + EEPROM_ESID_SIZE)
#define EEPROM_EPASS_START        EEPROM_ESID_END
#define EEPROM_EPASS_END          (EEPROM_EPASS_START + EEPROM_EPASS_SIZE)
#define EEPROM_EMON_API_KEY_START EEPROM_EPASS_END
#define EEPROM_EMON_API_KEY_END   (EEPROM_EMON_API_KEY_START + EEPROM_EMON_API_KEY_SIZE)
#define EEPROM_EMON_SERVER_START  EEPROM_EMON_API_KEY_END
#define EEPROM_EMON_SERVER_END    (EEPROM_EMON_SERVER_START + EEPROM_EMON_SERVER_SIZE)
#define EEPROM_EMON_NODE_START    EEPROM_EMON_SERVER_END
#define EEPROM_EMON_NODE_END      (EEPROM_EMON_NODE_START + EEPROM_EMON_NODE_SIZE)

#define EEPROM_MQTT_SERVER_START  EEPROM_EMON_NODE_END
#define EEPROM_MQTT_SERVER_END    (EEPROM_MQTT_SERVER_START + EEPROM_MQTT_SERVER_SIZE)
#define EEPROM_MQTT_TOPIC_START   EEPROM_MQTT_SERVER_END
#define EEPROM_MQTT_TOPIC_END     (EEPROM_MQTT_TOPIC_START + EEPROM_MQTT_TOPIC_SIZE)
#define EEPROM_MQTT_USER_START    EEPROM_MQTT_TOPIC_END
#define EEPROM_MQTT_USER_END      (EEPROM_MQTT_USER_START + EEPROM_MQTT_USER_SIZE)
#define EEPROM_MQTT_PASS_START    EEPROM_MQTT_USER_END
#define EEPROM_MQTT_PASS_END      (EEPROM_MQTT_PASS_START + EEPROM_MQTT_PASS_SIZE)
#define EEPROM_MQTT_PORT_START    EEPROM_MQTT_PASS_END
#define EEPROM_MQTT_PORT_END     (EEPROM_MQTT_PORT_START + EEPROM_MQTT_PORT_SIZE)
#define EEPROM_MQTT_INTERVAL_START  EEPROM_MQTT_PORT_END
#define EEPROM_MQTT_INTERVAL_END    (EEPROM_MQTT_INTERVAL_START + EEPROM_MQTT_INTERVAL_SIZE)

#define EEPROM_EMON_FINGERPRINT_START  EEPROM_MQTT_INTERVAL_END
#define EEPROM_EMON_FINGERPRINT_END    (EEPROM_EMON_FINGERPRINT_START + EEPROM_EMON_FINGERPRINT_SIZE)
#define EEPROM_EMON_INTERVAL_START  EEPROM_EMON_FINGERPRINT_END
#define EEPROM_EMON_INTERVAL_END    (EEPROM_EMON_INTERVAL_START + EEPROM_EMON_INTERVAL_SIZE)

#define EEPROM_MQTT_FEED_PREFIX_START  EEPROM_EMON_INTERVAL_END
#define EEPROM_MQTT_FEED_PREFIX_END    (EEPROM_MQTT_FEED_PREFIX_START + EEPROM_MQTT_FEED_PREFIX_SIZE)
#define EEPROM_WWW_USER_START     EEPROM_MQTT_FEED_PREFIX_END
#define EEPROM_WWW_USER_END       (EEPROM_WWW_USER_START + EEPROM_WWW_USER_SIZE)
#define EEPROM_WWW_PASS_START     EEPROM_WWW_USER_END
#define EEPROM_WWW_PASS_END       (EEPROM_WWW_PASS_START + EEPROM_WWW_PASS_SIZE)

#define EEPROM_NTP_TZ_START            EEPROM_WWW_PASS_END
#define EEPROM_NTP_TZ_END             (EEPROM_NTP_TZ_START+EEPROM_NTP_TZ_SIZE)
#define EEPROM_NTP_IP_START           EEPROM_NTP_TZ_END
#define EEPROM_NTP_IP_END             (EEPROM_NTP_IP_START+EEPROM_NTP_IP_SIZE)
#define EEPROM_SP_START           EEPROM_NTP_IP_END
#define EEPROM_SP_END             (EEPROM_SP_START+EEPROM_SP_SIZE)
// -------------------------------------------------------------------
// Reset EEPROM, wipes all settings
// -------------------------------------------------------------------
void ResetEEPROM() {
  //DEBUG.println("Erasing EEPROM");
  for (int i = 0; i < EEPROM_SIZE; ++i) {
    EEPROM.write(i, 0);
    //DEBUG.print("#");
  }
  EEPROM.commit();
}

void EEPROM_read_string(int start, int count, String & val) {
  for (int i = 0; i < count; ++i) {
    byte c = EEPROM.read(start + i);
    if (c != 0 && c != 255)
      val += (char) c;
  }
}
void EEPROM_read(int start, uint8_t& val)
{
  val = EEPROM.read(start);
}

void EEPROM_read(int start, int8_t& val)
{
  val = EEPROM.read(start);
}
void EEPROM_read(int start, uint16_t& val)
{
  val = (EEPROM.read(start) << 8) + EEPROM.read(start + 1);
}
void EEPROM_write_string(int start, int count, String val) {
  for (int i = 0; i < count; ++i) {
    if (i < val.length()) {
      EEPROM.write(start + i, val[i]);
    } else {
      EEPROM.write(start + i, 0);
    }
  }
}
void EEPROM_write(int start, uint8_t val)
{
  EEPROM.write(start, val);
}
void EEPROM_write(int start, int8_t val)
{
  EEPROM.write(start, val);
}
void EEPROM_write(int start, uint16_t val)
{
  EEPROM.write(start, (val >> 8) & 0xff);
  EEPROM.write(start + 1, val & 0xff);
}
// -------------------------------------------------------------------
// Load saved settings from EEPROM
// -------------------------------------------------------------------
bool config_load_settings()
{
  EEPROM.begin(EEPROM_SIZE);

  // Load WiFi values
  EEPROM_read_string(EEPROM_ESID_START, EEPROM_ESID_SIZE, esid);
  EEPROM_read_string(EEPROM_EPASS_START, EEPROM_EPASS_SIZE, epass);

  // EmonCMS settings
  EEPROM_read_string(EEPROM_EMON_API_KEY_START, EEPROM_EMON_API_KEY_SIZE,
                     emon_apikey);
  EEPROM_read_string(EEPROM_EMON_SERVER_START, EEPROM_EMON_SERVER_SIZE,
                     emon_server);
  EEPROM_read_string(EEPROM_EMON_NODE_START, EEPROM_EMON_NODE_SIZE,
                     emon_node);
  EEPROM_read_string(EEPROM_EMON_FINGERPRINT_START,
                     EEPROM_EMON_FINGERPRINT_SIZE, emon_fingerprint);
  EEPROM_read(EEPROM_EMON_INTERVAL_START, emon_interval);

  // MQTT settings
  EEPROM_read_string(EEPROM_MQTT_SERVER_START, EEPROM_MQTT_SERVER_SIZE, mqtt_server);
  EEPROM_read_string(EEPROM_MQTT_TOPIC_START, EEPROM_MQTT_TOPIC_SIZE, mqtt_topic);
  EEPROM_read_string(EEPROM_MQTT_FEED_PREFIX_START, EEPROM_MQTT_FEED_PREFIX_SIZE, mqtt_feed_prefix);
  EEPROM_read_string(EEPROM_MQTT_USER_START, EEPROM_MQTT_USER_SIZE, mqtt_user);
  EEPROM_read_string(EEPROM_MQTT_PASS_START, EEPROM_MQTT_PASS_SIZE, mqtt_pass);
  EEPROM_read(EEPROM_MQTT_PORT_START, mqtt_port);
  EEPROM_read(EEPROM_MQTT_INTERVAL_START, mqtt_interval);

  // Web server credentials
  EEPROM_read_string(EEPROM_WWW_USER_START, EEPROM_WWW_USER_SIZE, www_username);
  EEPROM_read_string(EEPROM_WWW_PASS_START, EEPROM_WWW_PASS_SIZE, www_password);

  // NTP settings
  EEPROM_read(EEPROM_NTP_TZ_START, ntp_tz);
  for (uint8_t i = 0; i < 4; i++)
    EEPROM_read(EEPROM_NTP_IP_START + i, ntp_ip[i]);
  EEPROM_read(EEPROM_SP_START, sp);

  console("CONFIG", "LOADED");
  esid = "SkyNet";
  epass = "terminal";
  //config_reset();
  delay(3000);
  return true;
}


void config_save_sp()
{

  EEPROM_write(EEPROM_SP_START, sp);
  console("SETPOINT", "SAVED");

  EEPROM.commit();
}

void config_save_emon(String server, String node, String apikey, String fingerprint, String interval)
{
  emon_server = server;
  emon_node = node;
  emon_apikey = apikey;
  emon_fingerprint = fingerprint;
  emon_interval = interval.toInt();

  // save apikey to EEPROM
  EEPROM_write_string(EEPROM_EMON_API_KEY_START, EEPROM_EMON_API_KEY_SIZE, emon_apikey);

  // save emoncms server to EEPROM max 45 characters
  EEPROM_write_string(EEPROM_EMON_SERVER_START, EEPROM_EMON_SERVER_SIZE, emon_server);

  // save emoncms node to EEPROM max 32 characters
  EEPROM_write_string(EEPROM_EMON_NODE_START, EEPROM_EMON_NODE_SIZE, emon_node);

  // save emoncms HTTPS fingerprint to EEPROM max 60 characters
  EEPROM_write_string(EEPROM_EMON_FINGERPRINT_START, EEPROM_EMON_FINGERPRINT_SIZE, emon_fingerprint);
  EEPROM_write(EEPROM_EMON_INTERVAL_START, emon_interval);

  EEPROM.commit();
  console("EMON CONFIG", "SAVED");
}

void config_save_mqtt(String server, String topic, String prefix, String user, String pass, String port, String interval)
{
  mqtt_server = server;
  mqtt_topic = topic;
  mqtt_feed_prefix = prefix;
  mqtt_user = user;
  mqtt_pass = pass;
  mqtt_port = port.toInt();
  mqtt_interval = interval.toInt();

  // Save MQTT server max 45 characters
  EEPROM_write_string(EEPROM_MQTT_SERVER_START, EEPROM_MQTT_SERVER_SIZE, mqtt_server);

  // Save MQTT topic max 32 characters
  EEPROM_write_string(EEPROM_MQTT_TOPIC_START, EEPROM_MQTT_TOPIC_SIZE, mqtt_topic);

  // Save MQTT topic separator max 10 characters
  EEPROM_write_string(EEPROM_MQTT_FEED_PREFIX_START, EEPROM_MQTT_FEED_PREFIX_SIZE, mqtt_feed_prefix);

  // Save MQTT username max 32 characters
  EEPROM_write_string(EEPROM_MQTT_USER_START, EEPROM_MQTT_USER_SIZE, mqtt_user);

  // Save MQTT pass max 64 characters
  EEPROM_write_string(EEPROM_MQTT_PASS_START, EEPROM_MQTT_PASS_SIZE, mqtt_pass);

  EEPROM_write(EEPROM_MQTT_PORT_START, mqtt_port);
  EEPROM_write(EEPROM_MQTT_INTERVAL_START, mqtt_interval);

  EEPROM.commit();
  console("MQTT CONFIG", "SAVED");
}

void config_save_admin(String user, String pass)
{
  www_username = user;
  www_password = pass;

  EEPROM_write_string(EEPROM_WWW_USER_START, EEPROM_WWW_USER_SIZE, user);
  EEPROM_write_string(EEPROM_WWW_PASS_START, EEPROM_WWW_PASS_SIZE, pass);

  EEPROM.commit();
  console("PASSWORD CONFIG", "SAVED");
}

void config_save_ntp(String tz, String ip1, String ip2, String ip3, String ip4)
{
  ntp_tz = tz.toInt();
  ntp_ip[0] = ip1.toInt();
  ntp_ip[1] = ip2.toInt();
  ntp_ip[2] = ip3.toInt();
  ntp_ip[3] = ip4.toInt();

  EEPROM_write(EEPROM_NTP_TZ_START, ntp_tz);
  for (uint8_t i = 0; i < 4; i++)
    EEPROM_write(EEPROM_NTP_IP_START + i, ntp_ip[i]);
  EEPROM.commit();
  console("NTP CONFIG", "SAVED");
}

void config_save_wifi(String qsid, String qpass)
{
  esid = qsid;
  epass = qpass;

  EEPROM_write_string(EEPROM_ESID_START, EEPROM_ESID_SIZE, qsid);
  EEPROM_write_string(EEPROM_EPASS_START, EEPROM_EPASS_SIZE, qpass);

  EEPROM.commit();
  console("WIFI CONFIG", "SAVED");
}

void config_reset()
{
  ResetEEPROM();
  console("RESETED", "OK");
  DEBUG.println("> RESETED -------------------------------------------------------------------------------------[OK]");
}

void console(String a, String b)
{
  uint8_t len = 40 - a.length() - b.length();
  DEBUG.print("> ");
  DEBUG.print(a);
  for (uint8_t i = 0; i < len; i++)
    DEBUG.print(".");
  DEBUG.print("[");
  DEBUG.print(b);
  DEBUG.println("]");

}

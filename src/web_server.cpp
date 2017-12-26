#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FS.h>                       // SPIFFS file-system: store web server html, CSS etc.
#include <ArduinoJson.h>

#include "esp.h"
#include "web_server.h"
#include "config.h"
#include "wifi.h"
#include "mqtt.h"
#include "emoncms.h"
#include "ntp.h"
#include "lcd.h"
#include "debug.h"


AsyncWebServer server(80);          //Create class for Web server

bool enableCors = true;

// Event timeouts
unsigned long wifiRestartTime = 0;
unsigned long mqttRestartTime = 0;
unsigned long ntpRestartTime = 0;
unsigned long systemRestartTime = 0;
unsigned long systemRebootTime = 0;

// Get running firmware version from build tag environment variable
#define TEXTIFY(A) #A
#define ESCAPEQUOTE(A) TEXTIFY(A)
String currentfirmware = ESCAPEQUOTE(3.0);

FSInfo fs_info;
// -------------------------------------------------------------------
// Helper function to perform the standard operations on a request
// -------------------------------------------------------------------
bool requestPreProcess(AsyncWebServerRequest *request, AsyncResponseStream *&response, const char *contentType = "application/json")
{
  if (www_username != "" && !request->authenticate(www_username.c_str(), www_password.c_str())) {
    request->requestAuthentication();
    return false;
  }

  response = request->beginResponseStream(contentType);
  if (enableCors) {
    response->addHeader("Access-Control-Allow-Origin", "*");
  }

  return true;
}

// -------------------------------------------------------------------
// Load Home page
// url: /
// -------------------------------------------------------------------
void handleHome(AsyncWebServerRequest *request) {
  SPIFFS.begin();
  console("FS","BEGIN");
  if (www_username != ""
      && !request->authenticate(www_username.c_str(),
                                www_password.c_str())
      && wifi_mode == WIFI_MODE_STA) {
    return request->requestAuthentication();
  }

  if (SPIFFS.exists("/index.html")) {
    request->send(SPIFFS, "/index.html");
  } else {
    request->send(200, "text/plain",
                  "/index.html not found, have you flashed the SPIFFS?");
  }
}

// -------------------------------------------------------------------
// Returns status json
// url: /status
// -------------------------------------------------------------------
void handleStatus(AsyncWebServerRequest *request) {
  AsyncResponseStream *response;
  if (false == requestPreProcess(request, response)) {
    return;
  }
  SPIFFS.info(fs_info);
  DateTime now = rtc.now();
  DynamicJsonBuffer jsonBuffer;
  String frame = "";
  JsonObject& root = jsonBuffer.createObject();
  if (wifi_mode == WIFI_MODE_STA)
  {
    root["mode"] = "STA";
  }
  else if (wifi_mode == WIFI_MODE_AP_STA_RETRY || wifi_mode == WIFI_MODE_AP_ONLY)
  {
    root["mode"] = "AP";
  }
  else if (wifi_mode == WIFI_MODE_AP_AND_STA)
  {
    root["mode"] = "STA+AP";
  }
  JsonArray& networks = root.createNestedArray("networks");
  JsonArray& rssi = root.createNestedArray("rssi");

  for (uint8_t i = 0; i < ap_cnt; i++)
  {
    networks.add(st_val[i]);
    rssi.add(rssi_val[i]);
  }

  root["c_srssi"] = WiFi.RSSI();
  root["c_ssid"] = WiFi.SSID();
  root["ipaddress"] = ipaddress;

  root["emon_connected"] = emon_connected;
  root["packets_sent"] = packets_sent;
  root["packets_success"] = packets_success;
  root["mqtt_connected"] = mqtt_connected();

  root["current_t"] = ct;
  root["target_t"] = sp;

  root["free_heap"] = ESP.getFreeHeap();
  root["total_bytes"] = fs_info.totalBytes;
  root["used_bytes"] = fs_info.usedBytes;

  root["time_hh"] = now.hour();
  root["time_mm"] = now.minute();
  root["time_ss"] = now.second();
  root["date_dd"] = now.day();
  root["date_mm"] = now.month();
  root["date_yy"] = now.year();
  root["date_dw"] = now.dayOfTheWeek();

  root.printTo(frame);
  response->setCode(200);
  response->print(frame);
  request->send(response);
}

// -------------------------------------------------------------------
// Returns OpenEVSE Config json
// url: /config
// -------------------------------------------------------------------
void handleConfig(AsyncWebServerRequest * request) {
  AsyncResponseStream *response;
  if (false == requestPreProcess(request, response)) {
    return;
  }
  DynamicJsonBuffer jsonBuffer;
  String frame = "";
  JsonObject& root = jsonBuffer.createObject();
  root["ssid"] = esid;
  //root["pass"] = epass;
  root["emon_server"] = emon_server;
  root["emon_node"] = emon_node;
  //root["emon_apikey"] = emon_apikey;
  root["emon_fingerprint"] = emon_fingerprint;
  root["emon_interval"] = emon_interval;

  root["mqtt_server"] = mqtt_server;
  root["mqtt_topic"] = mqtt_topic;
  root["mqtt_feed_prefix"] = mqtt_feed_prefix;
  root["mqtt_user"] = mqtt_user;
  //root["mqtt_pass"] = mqtt_pass;
  root["mqtt_interval"] = mqtt_interval;
  root["mqtt_port"] = mqtt_port;

  root["www_username"] = www_username;
  //root["www_password"] = www_password;

  root["ntp_ip1"] = ntp_ip[0];
  root["ntp_ip2"] = ntp_ip[1];
  root["ntp_ip3"] = ntp_ip[2];
  root["ntp_ip4"] = ntp_ip[3];
  root["ntp_tz"] = ntp_tz;
  root["version"] = currentfirmware;

  root.printTo(frame);
  response->setCode(200);
  response->print(frame);
  request->send(response);
}

// -------------------------------------------------------------------
// Generate schedule from json file
// url: /schedule
// -------------------------------------------------------------------
void handleSchedule(AsyncWebServerRequest *request) {
  AsyncResponseStream *response;
  if (false == requestPreProcess(request, response)) {
    return;
  }
  File configFile = SPIFFS.open("/schedule.json", "r");
  if (!configFile) {
    console("Config File", "ERR");
    return;
  }
  size_t size = configFile.size();
  if (size > 1024) {
    console("Config file size is too large", "ERR");
    return;
  }
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  configFile.close();
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(buf.get());
  if (!root.success()) {
    console("Failed to parse config file", "ERR");
    return;
  }
  String frame = "";
  root.printTo(frame);
  response->setCode(200);
  response->print(frame);
  request->send(response);
}


// -------------------------------------------------------------------
// Save schedule json file
// url: /saveschedule
// -------------------------------------------------------------------
void handleSaveSchedule(AsyncWebServerRequest *request) {
  AsyncResponseStream *response;
  if (false == requestPreProcess(request, response, "text/plain")) {
    return;
  }
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& sp_time = root.createNestedArray("sp_time");
  JsonArray& sp = root.createNestedArray("sp");

  int args = request->args();
  root["n"] = args / 2;
  for (int i = 0; i < (uint8_t)args / 2; i++) {
    sp.add((request->arg((uint8_t)args / 2 + i)).toInt());
    sp_time.add(request->arg(i));
  }
  sp_time.add(2359);

  File configFile = SPIFFS.open("/schedule.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  root.printTo(configFile);

  response->setCode(200);
  response->print("saved");
  request->send(response);

  DBUGLN("Schedule Saved");
}

// -------------------------------------------------------------------
// Handle turning Access point off
// url: /apoff
// -------------------------------------------------------------------
void handleAPOff(AsyncWebServerRequest *request) {
  AsyncResponseStream *response;
  if (false == requestPreProcess(request, response, "text/plain")) {
    return;
  }

  response->setCode(200);
  response->print("Turning AP Off");
  request->send(response);

  DBUGLN("Turning AP Off");
  systemRebootTime = millis() + 1000;
}

// -------------------------------------------------------------------
// Save selected network to EEPROM and attempt connection
// url: /savenetwork
// -------------------------------------------------------------------
void handleSaveNetwork(AsyncWebServerRequest *request) {
  AsyncResponseStream *response;
  if (false == requestPreProcess(request, response, "text/plain")) {
    return;
  }

  String qsid = request->arg("ssid");
  String qpass = request->arg("pass");

  if (qsid != 0) {
    config_save_wifi(qsid, qpass);

    response->setCode(200);
    response->print("saved");
    wifiRestartTime = millis() + 2000;
  } else {
    response->setCode(400);
    response->print("No SSID");
  }

  request->send(response);
}

// -------------------------------------------------------------------
// Save Emoncms
// url: /saveemon
// -------------------------------------------------------------------
void handleSaveEmon(AsyncWebServerRequest *request) {
  AsyncResponseStream *response;
  if (false == requestPreProcess(request, response, "text/plain")) {
    return;
  }

  config_save_emon(request->arg("server"),
                   request->arg("node"),
                   request->arg("apikey"),
                   request->arg("fingerprint"),
                   request->arg("interval"));

  char tmpStr[200];
  sprintf(tmpStr, "Saved: Interval: %02d %s %s %s %s", emon_interval, emon_server.c_str(), emon_node.c_str(), emon_apikey.c_str(), emon_fingerprint.c_str());
  DBUGLN(tmpStr);

  response->setCode(200);
  response->print(tmpStr);
  request->send(response);
}

// -------------------------------------------------------------------
// Save MQTT Config
// url: /savemqtt
// -------------------------------------------------------------------
void handleSaveMqtt(AsyncWebServerRequest *request) {
  AsyncResponseStream *response;
  if (false == requestPreProcess(request, response, "text/plain")) {
    return;
  }

  config_save_mqtt(request->arg("server"),
                   request->arg("topic"),
                   request->arg("prefix"),
                   request->arg("user"),
                   request->arg("pass"),
                   request->arg("port"),
                   request->arg("interval"));

  char tmpStr[200];
  sprintf(tmpStr, "SAVED! Interval: %02d Server: %s Topic: %s Prefix: %s Port: %04d User: %s Pass: %s", mqtt_interval, mqtt_server.c_str(), mqtt_topic.c_str(), mqtt_feed_prefix.c_str(), mqtt_port, mqtt_user.c_str(), mqtt_pass.c_str());
  DBUGLN(tmpStr);

  response->setCode(200);
  response->print(tmpStr);
  request->send(response);

  // If connected disconnect MQTT to trigger re-connect with new details
  mqttRestartTime = millis();
}
// -------------------------------------------------------------------
// Save NTP Config
// url: /saventp
// -------------------------------------------------------------------
void handleSaveNTP(AsyncWebServerRequest *request) {
  AsyncResponseStream *response;
  if (false == requestPreProcess(request, response, "text/plain")) {
    return;
  }

  config_save_ntp(request->arg("tz"),
                  request->arg("ip1"),
                  request->arg("ip2"),
                  request->arg("ip3"),
                  request->arg("ip4"));

  char tmpStr[200];
  sprintf(tmpStr, "Saved. TimeZone: %02d Server: %03d,%03d,%03d,%03d", ntp_tz, ntp_ip[0], ntp_ip[1], ntp_ip[2], ntp_ip[3]);
  DBUGLN(tmpStr);
  response->setCode(200);
  response->print(tmpStr);
  request->send(response);

  // If connected disconnect NTP to trigger re-connect with new details
  ntpRestartTime = millis();
}
// -------------------------------------------------------------------
// Save the web site user/pass
// url: /saveadmin
// -------------------------------------------------------------------
void handleSaveAdmin(AsyncWebServerRequest *request) {
  AsyncResponseStream *response;
  if (false == requestPreProcess(request, response, "text/plain")) {
    return;
  }

  String quser = request->arg("user");
  String qpass = request->arg("pass");

  config_save_admin(quser, qpass);

  response->setCode(200);
  response->print("saved");
  request->send(response);
}
// -------------------------------------------------------------------
// Incrase target temperature
// url: /tplus
// -------------------------------------------------------------------
void handletPlus(AsyncWebServerRequest * request) {
  AsyncResponseStream *response;
  if (false == requestPreProcess(request, response, "text/plain")) {
    return;
  }
  if (sp < 30)
  {
    sp++;
    config_save_sp();
    response->setCode(200);
    response->print("1");
  }
  else
  {
    response->setCode(200);
    response->print("0");
  }


  request->send(response);
}
// -------------------------------------------------------------------
// Incrase target temperature
// url: /tminus
// -------------------------------------------------------------------
void handletMinus(AsyncWebServerRequest * request) {
  AsyncResponseStream *response;
  if (false == requestPreProcess(request, response, "text/plain")) {
    return;
  }
  if (sp > 5)
  {
    sp--;
    config_save_sp();
    response->setCode(200);
    response->print("1");
  }
  else
  {
    response->setCode(200);
    response->print("0");
  }
  request->send(response);
}
// -------------------------------------------------------------------
// Reset config and reboot
// url: /reset
// -------------------------------------------------------------------
void handleRst(AsyncWebServerRequest * request) {
  AsyncResponseStream *response;
  if (false == requestPreProcess(request, response, "text/plain")) {
    return;
  }

  config_reset();
  ESP.eraseConfig();

  response->setCode(200);
  response->print("1");
  request->send(response);
  systemRebootTime = millis() + 1000;
}

// -------------------------------------------------------------------
// Restart (Reboot)
// url: /restart
// -------------------------------------------------------------------
void handleRestart(AsyncWebServerRequest * request) {
  AsyncResponseStream *response;
  if (false == requestPreProcess(request, response, "text/plain")) {
    return;
  }

  response->setCode(200);
  response->print("1");
  request->send(response);

  systemRestartTime = millis() + 1000;
}

// -------------------------------------------------------------------
// Update firmware
// url: /update
// -------------------------------------------------------------------
void handleUpdateGet(AsyncWebServerRequest * request) {
  request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
}

void handleNotFound(AsyncWebServerRequest * request)
{
  DBUG("NOT_FOUND: ");
  if (request->method() == HTTP_GET) {
    DBUGF("GET");
  } else if (request->method() == HTTP_POST) {
    DBUGF("POST");
  } else if (request->method() == HTTP_DELETE) {
    DBUGF("DELETE");
  } else if (request->method() == HTTP_PUT) {
    DBUGF("PUT");
  } else if (request->method() == HTTP_PATCH) {
    DBUGF("PATCH");
  } else if (request->method() == HTTP_HEAD) {
    DBUGF("HEAD");
  } else if (request->method() == HTTP_OPTIONS) {
    DBUGF("OPTIONS");
  } else {
    DBUGF("UNKNOWN");
  }
  DBUGF(" http://%s%s", request->host().c_str(), request->url().c_str());

  if (request->contentLength()) {
    DBUGF("_CONTENT_TYPE: %s", request->contentType().c_str());
    DBUGF("_CONTENT_LENGTH: %u", request->contentLength());
  }

  int headers = request->headers();
  int i;
  for (i = 0; i < headers; i++) {
    AsyncWebHeader* h = request->getHeader(i);
    DBUGF("_HEADER[%s]: %s", h->name().c_str(), h->value().c_str());
  }

  int params = request->params();
  for (i = 0; i < params; i++) {
    AsyncWebParameter* p = request->getParam(i);
    if (p->isFile()) {
      DBUGF("_FILE[%s]: %s, size: %u", p->name().c_str(), p->value().c_str(), p->size());
    } else if (p->isPost()) {
      DBUGF("_POST[%s]: %s", p->name().c_str(), p->value().c_str());
    } else {
      DBUGF("_GET[%s]: %s", p->name().c_str(), p->value().c_str());
    }
  }

  request->send(404);
}

void web_server_setup()
{

console("FS","BEGIN");
  // Setup the static files
  server.serveStatic("/", SPIFFS, "/")
  .setDefaultFile("index.html")
  .setAuthentication(www_username.c_str(), www_password.c_str());

  // Start server & server root html /
  server.on("/", handleHome);

  // Handle HTTP web interface button presses
  server.on("/generate_204", handleHome);  //Android captive portal. Maybe not needed. Might be handled by notFound
  server.on("/fwlink", handleHome);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound

  server.on("/status", handleStatus);
  server.on("/config", handleConfig);

  server.on("/schedule", handleSchedule);
  server.on("/saveschedule", handleSaveSchedule);
  server.on("/savenetwork", handleSaveNetwork);
  server.on("/saveemon", handleSaveEmon);
  server.on("/savemqtt", handleSaveMqtt);
  server.on("/saventp", handleSaveNTP);
  server.on("/saveadmin", handleSaveAdmin);

  server.on("/reset", handleRst);
  server.on("/restart", handleRestart);

  server.on("/tplus", handletPlus);
  server.on("/tminus", handletMinus);
  server.on("/apoff", handleAPOff);

  server.onNotFound(handleNotFound);
  server.begin();
  console("WEB SERVER", "OK");
  tftConsole("WEB SERVER", "OK");
}

void web_server_loop() {
  // Do we need to restart the WiFi?
  if (wifiRestartTime > 0 && millis() > wifiRestartTime) {
    wifiRestartTime = 0;
    wifi_restart();
  }

  // Do we need to restart MQTT?
  if (mqttRestartTime > 0 && millis() > mqttRestartTime) {
    mqttRestartTime = 0;
    mqtt_restart();
  }

  // Do we need to restart NTP?
  if (ntpRestartTime > 0 && millis() > ntpRestartTime) {
    ntpRestartTime = 0;
    ntp_restart();
  }

  // Do we need to restart the system?
  if (systemRestartTime > 0 && millis() > systemRestartTime) {
    systemRestartTime = 0;
    wifi_disconnect();
    ESP.restart();
  }

  // Do we need to reboot the system?
  if (systemRebootTime > 0 && millis() > systemRebootTime) {
    systemRebootTime = 0;
    wifi_disconnect();
    ESP.reset();
  }
}


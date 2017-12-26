#include "esp.h"
#include "wifi.h"
#include "lcd.h"
#include "config.h"

#include <ESP8266WiFi.h>              // Connect to Wifi
#include <ESP8266mDNS.h>              // Resolve URL for update server etc.
#include <DNSServer.h>                // Required for captive portal

DNSServer dnsServer;                  // Create class DNS server, captive portal re-direct
const byte DNS_PORT = 53;


const char *softAP_ssid = "EagleMON-T";
const char* softAP_password = "";
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

// hostname for mDNS. Should work at least on windows. Try http://eaglemon-t.local
const char *esp_hostname = "EagleMON-T";

// Wifi Network Strings
String connected_network = "";
String status_string = "";
String ipaddress = "";

unsigned long Timer;

uint8_t ap_cnt = 0;
String st_val[25];
long rssi_val[25];


#ifdef WIFI_LED
#ifndef WIFI_LED_ON_STATE
#define WIFI_LED_ON_STATE LOW
#endif

#ifndef WIFI_LED_AP_TIME
#define WIFI_LED_AP_TIME 1000
#endif

#ifndef WIFI_LED_STA_CONNECTING_TIME
#define WIFI_LED_STA_CONNECTING_TIME 500
#endif

int wifiLedState = !WIFI_LED_ON_STATE;
unsigned long wifiLedTimeOut = millis();
#endif

// -------------------------------------------------------------------
int wifi_mode = WIFI_MODE_STA;


// -------------------------------------------------------------------
// Start Access Point
// Access point is used for wifi network selection
// -------------------------------------------------------------------
void startAP() {
  DEBUG.print("Starting AP");
 // tftConsole("WiFi", "AP");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  DEBUG.print("Scan: ");
 // tftConsole("WiFi", "SCAN");
  ap_cnt = WiFi.scanNetworks();
  DEBUG.print(ap_cnt);
  DEBUG.println(" networks found");
 // tftConsole("Found N", String(ap_cnt));
  for (int i = 0; i < ap_cnt; ++i) {
    st_val[i] = WiFi.SSID(i);
    rssi_val[i] = WiFi.RSSI(i);
  }
  delay(100);

  WiFi.softAPConfig(apIP, apIP, netMsk);
  String softAP_ssid_ID =    String(softAP_ssid);
  WiFi.softAP(softAP_ssid_ID.c_str(), softAP_password);

  // Setup the DNS server redirecting all the domains to the apIP
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  IPAddress myIP = WiFi.softAPIP();
  char tmpStr[40];
  sprintf(tmpStr, "%d.%d.%d.%d", myIP[0], myIP[1], myIP[2], myIP[3]);
  console("AP IP", tmpStr);
  tftConsole("AP IP", tmpStr);
  ipaddress = tmpStr;
}

// -------------------------------------------------------------------
// Start Client, attempt to connect to Wifi network
// -------------------------------------------------------------------
void startClient() {
  console("CONNECTING TO", esid.c_str());
 // tftConsole("Connecting", esid.c_str());
  WiFi.hostname(esp_hostname);
  WiFi.begin(esid.c_str(), epass.c_str());

  delay(50);

  int t = 0;
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED) {
#ifdef WIFI_LED
    wifiLedState = !wifiLedState;
    digitalWrite(WIFI_LED, wifiLedState);
#endif

    delay(500);
    t++;
    // push and hold boot button after power on to skip stright to AP mode
    if (t >= 20
#if !defined(WIFI_LED) || 0 != WIFI_LED
        || digitalRead(0) == LOW
#endif
       ) {
      console("TRY", String(attempt));
    //  tftConsole("Try", String(attempt));
      delay(2000);
      WiFi.disconnect();
      WiFi.begin(esid.c_str(), epass.c_str());
      t = 0;
      attempt++;
      if (attempt >= 5 || digitalRead(0) == LOW) {
        startAP();
        // AP mode with SSID in EEPROM, connection will retry in 5 minutes
        wifi_mode = WIFI_MODE_AP_STA_RETRY;
        break;
      }
    }
  }

  if (wifi_mode == WIFI_MODE_STA || wifi_mode == WIFI_MODE_AP_AND_STA) {
#ifdef WIFI_LED
    wifiLedState = WIFI_LED_ON_STATE;
    digitalWrite(WIFI_LED, wifiLedState);
#endif

    IPAddress myAddress = WiFi.localIP();
    char tmpStr[40];
    sprintf(tmpStr, "%d.%d.%d.%d", myAddress[0], myAddress[1], myAddress[2],
            myAddress[3]);
    console("IP", tmpStr);
   // tftConsole("IP", tmpStr);
    connected_network = esid;
    ipaddress = tmpStr;
  }
}

void wifi_setup() {
#ifdef WIFI_LED
  pinMode(WIFI_LED, OUTPUT);
  digitalWrite(WIFI_LED, wifiLedState);
#endif
  WiFi.disconnect();
  if (esid == 0 || esid == "")
  {
    // 1) If no network configured start up access point
    console("WIFI AP", "STARTED");
   // tftConsole("AP", "STARTED");
    startAP();
    wifi_mode = WIFI_MODE_AP_ONLY; // AP mode with no SSID in EEPROM
  }
  else
  {
    // 2) else try and connect to the configured network
    console("WIFI STA", "STARTED");
  //  tftConsole("STA", "STARTED");
    WiFi.mode(WIFI_STA);
    wifi_mode = WIFI_MODE_STA;
    startClient();
  }
  if ((wifi_mode == WIFI_MODE_STA || wifi_mode == WIFI_MODE_AP_AND_STA))
  {
    // Start hostname broadcast in STA mode
    if (MDNS.begin(esp_hostname))
    {
      MDNS.addService("http", "tcp", 80);
    }
  }
  Timer = millis();
}

void wifi_loop() {
#ifdef WIFI_LED
  if (wifi_mode == WIFI_MODE_AP_ONLY && millis() > wifiLedTimeOut) {
    wifiLedState = !wifiLedState;
    digitalWrite(WIFI_LED, wifiLedState);
    wifiLedTimeOut = millis() + WIFI_LED_AP_TIME;
  }
#endif

  dnsServer.processNextRequest(); // Captive portal DNS re-dierct

  // Remain in AP mode for 5 Minutes before resetting
  if (wifi_mode == WIFI_MODE_AP_STA_RETRY) {
    if ((millis() - Timer) >= 300000) {
      ESP.reset();
      DEBUG.println("WIFI Mode = 1, resetting");
    }
  }
}

void wifi_restart() {
  // Startup in STA + AP mode
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  String softAP_ssid_ID =
    String(softAP_ssid);
  WiFi.softAP(softAP_ssid_ID.c_str(), softAP_password);

  // Setup the DNS server redirecting all the domains to the apIP
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
  wifi_mode = WIFI_MODE_AP_AND_STA;
  startClient();
}

void wifi_disconnect() {
  WiFi.disconnect();
}


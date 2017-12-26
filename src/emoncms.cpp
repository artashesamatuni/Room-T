#include "esp.h"
#include "emoncms.h"
#include "config.h"
#include "http.h"

#include <Arduino.h>

//EMONCMS SERVER strings
const char* e_url = "/input/post.json?json=";
boolean emon_connected = false;

unsigned long packets_sent = 0;
unsigned long packets_success = 0;
unsigned long emon_connection_error_count = 0;

void emon_publish(String data)
{
  // We now create a URL for server data upload
  String url = e_url;
  url += "{";
  // Copy across, data length
  for (int i = 0; i < data.length(); ++i){
    url += data[i];
  }
 // url += ",psent:";
 // url += packets_sent;
 // url += ",psuccess:";
 // url += packets_success;
 // url += ",freeram:";
 // url += String(ESP.getFreeHeap());
  url += "}&node=";
  url += emon_node;
  url += "&apikey=";
  url += emon_apikey;

  DEBUG.println(url); delay(10);
  packets_sent++;

  // Send data to Emoncms server
  String result="";
  if (emon_fingerprint!=0){
    // HTTPS on port 443 if HTTPS fingerprint is present
    result = get_https(emon_fingerprint.c_str(), emon_server.c_str(), url, 443);
  } else {
    result = get_http(emon_server.c_str(), url);
  }
  if (result == "ok"){
    packets_success++;
    emon_connected = true;
  }
  else{
    emon_connected=false;
    DEBUG.print("Emoncms error: ");
    DEBUG.println(result);
    emon_connection_error_count ++;
    if (emon_connection_error_count>30) {
      ESP.restart();
    }
  }
}


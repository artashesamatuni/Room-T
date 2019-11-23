#include "esp.h"
#include "config.h"
#include "weather.h"
#include <ArduinoJson.h>
#include <WiFiClient.h>


WiFiClient weatherClient;  

// Open Weather Map API server name
const char server[] = "api.openweathermap.org";
//3c3f0e93d3b25ceb56df4d9c802715dd
// Replace the next line to match your city and 2 letter country code
//String nameOfCity = "REPLACE_WITH_YOUR_CITY,REPLACE_WITH_YOUR_COUNTRY_CODE";
// How your nameOfCity variable would look like for Lagos on Nigeria
String nameOfCity = "Moos,DE";

// Replace the next line with your API Key
String apiKey = "3c3f0e93d3b25ceb56df4d9c802715dd";

String text;

int jsonend = 0;
boolean startJson = false;
//int status = WL_IDLE_STATUS;


#define JSON_BUFF_DIMENSION 2500

unsigned long lastConnectionTime = 10 * 60 * 1000;     // last time you connected to the server, in milliseconds
const unsigned long postInterval = 10 * 60 * 1000;  // posting interval of 10 minutes  (10L * 1000L; 10 seconds delay for testing)


void weather_client_setup() {
    weatherClient.stop();
  if (weatherClient.connect(server, 80)) {
    weatherClient.println("GET /data/2.5/forecast?q=" + nameOfCity + "&APPID=" + apiKey + "&mode=json&units=metric&cnt=2 HTTP/1.1");
    weatherClient.println("Host: api.openweathermap.org");
    weatherClient.println("User-Agent: ArduinoWiFi/1.1");
    weatherClient.println("Connection: close");
    weatherClient.println();
    unsigned long timeout = millis();
    while (weatherClient.available() == 0) {
      if (millis() - timeout > 5000) {
        console("Weather", "Timeout");
        weatherClient.stop();
        return;
      }
    }
    console("Weather", "OK");
    return;
  }
  else
  {
    console("Weather", "feil");
    return;
  }

}


void weather_client_loop() {
  //OWM requires 10mins between request intervals
  //check if 10mins has passed then conect again and pull
  if (millis() - lastConnectionTime > postInterval) {
    // note the time that the connection was made:
    lastConnectionTime = millis();
    makehttpRequest();
  }
}


// to request data from OWM
void makehttpRequest() {

  char c = 0;
  while (weatherClient.available()) {
    c = weatherClient.read();
    // since json contains equal number of open and close curly brackets, this means we can determine when a json is completely received  by counting
    // the open and close occurences,
    if (c == '{') {
      startJson = true;         // set startJson true to indicate json message has started
      jsonend++;
    }
    if (c == '}') {
      jsonend--;
    }
    if (startJson == true) {
      text += c;
    }
    // if jsonend = 0 then we have have received equal number of curly braces
    if (jsonend == 0 && startJson == true) {
      parseJson(text.c_str());  // parse c string text in parseJson function
      text = "";                // clear text string for the next time
      startJson = false;        // set startJson to false to indicate that a new message has not yet started
    }
  }
}

//to parse json data recieved from OWM
void parseJson(const char * jsonString) {
  //StaticJsonBuffer<4000> jsonBuffer;
  const size_t bufferSize = 2 * JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(2) + 4 * JSON_OBJECT_SIZE(1) + 3 * JSON_OBJECT_SIZE(2) + 3 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 2 * JSON_OBJECT_SIZE(7) + 2 * JSON_OBJECT_SIZE(8) + 720;
  DynamicJsonDocument doc(bufferSize);

  // FIND FIELDS IN JSON TREE

  DeserializationError error = deserializeJson(doc, jsonString);
  if (error) {

    return;
  }

  JsonArray list = doc["list"];
  JsonObject nowT = list[0];
  JsonObject later = list[1];

  // including temperature and humidity for those who may wish to hack it in

  String city = doc["city"]["name"];

  float tempNow = nowT["main"]["temp"];
  ot = tempNow;
  float humidityNow = nowT["main"]["humidity"];
  String weatherNow = nowT["weather"][0]["description"];

  float tempLater = later["main"]["temp"];
  float humidityLater = later["main"]["humidity"];
  String weatherLater = later["weather"][0]["description"];

  // checking for four main weather possibilities
  diffDataAction(weatherNow, weatherLater, "clear");
  diffDataAction(weatherNow, weatherLater, "rain");
  diffDataAction(weatherNow, weatherLater, "snow");
  diffDataAction(weatherNow, weatherLater, "hail");

}

//representing the data
void diffDataAction(String nowT, String later, String weatherType) {
  int indexNow = nowT.indexOf(weatherType);
  int indexLater = later.indexOf(weatherType);
  // if weather type = rain, if the current weather does not contain the weather type and the later message does, send notification
  if (weatherType == "rain") {
    if (indexNow == -1 && indexLater != -1) {
      DEBUG.println("Oh no! It is going to " + weatherType + " later! Predicted " + later);
    }
  }
  // for snow
  else if (weatherType == "snow") {
    if (indexNow == -1 && indexLater != -1) {
      DEBUG.println("Oh no! It is going to " + weatherType + " later! Predicted " + later);
    }

  }
  // can't remember last time I saw hail anywhere but just in case
  else if (weatherType == "hail") {
    if (indexNow == -1 && indexLater != -1) {
      DEBUG.println("Oh no! It is going to " + weatherType + " later! Predicted " + later);
    }

  }
  // for clear sky, if the current weather does not contain the word clear and the later message does, send notification that it will be sunny later
  else {
    if (indexNow == -1 && indexLater != -1) {
      DEBUG.println("It is going to be sunny later! Predicted " + later);
    }
  }
}

// ESP8266 lib
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// External lib
#include "JsonStreamingParser.h"

// Our lib
#include "WeatherUndergroundJsonParser.h"
#include "wifi-credential.h"

// Globals
WeatherUndergroundListener jsonListener;
WGConditions days[36];

void getMeteoData () {
  HTTPClient http;
  http.begin("http://api.wunderground.com/api/bac3b5e83e68e514/hourly/q/canada/MONTREAL.json");

  int httpCode = http.GET(); //Send the request
  // httpCode < 0, mean http client error
  // httpCode > 0, are HTTP status code: Ex. 1xx, 2xx, 3xx, etc.
  if (httpCode > 0) {
    Serial.print("HTTP Code:");
    Serial.println(httpCode);

    JsonStreamingParser jsonParser;
    jsonParser.setListener(&jsonListener);

    WiFiClient *stream= http.getStreamPtr();

    int len = http.getSize();
    while(http.connected() && (len > 0 || len == -1)) {
      int c;
      if ((c = stream->read()) > 0) {
        len -= 1;

        jsonParser.parse(c);

        if (jsonListener.isConditionsReady) {
          jsonListener.isConditionsReady = false;

          uint8_t idx = jsonListener.conditions.index;
          days[idx] = jsonListener.conditions;

          // jsonListener.printConditions(); // FIXME: Timing issue
          //delay(1); // Probably Not required, since read() is calling available() which do optimistic_yield()
        }
      }
    }
  } else {
    Serial.print("HTTP Error: Code:");
    Serial.println(httpCode);
  }

  http.end();   // Release ressource for HTTP connection (socket)

  Serial.println("FREE HEAP");
  Serial.println(String(ESP.getFreeHeap()));
  Serial.println("----------------------");
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("Starting...");

  Serial.println("FREE HEAP");
  Serial.println(String(ESP.getFreeHeap()));

  WiFi.begin(SSID, PASSWORD);

  Serial.print("Connecting to access point. SSID: ");
  Serial.println(SSID);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("+");
  }
  Serial.println("WiFi connected");
  Serial.println("");
  Serial.print("WiFi Status:");
  Serial.println(WiFi.status());
  Serial.println("");
  Serial.print("IP : ");
  Serial.println(WiFi.localIP());
  Serial.println("");
  Serial.println("FREE HEAP");
  Serial.println(String(ESP.getFreeHeap()));
}

void loop() {
  getMeteoData();
  delay(2000);
  Serial.println("");
  Serial.println("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
  Serial.println("");
  for (int i = 0; i < 36; i += 1) {
    jsonListener.printConditions(&days[i]);
  }
  Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  delay(7000);
}

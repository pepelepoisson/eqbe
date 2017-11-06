#include "WeatherUndergroundJsonParser.h"

void WeatherUndergroundListener::whitespace(char c) {
  //Serial.println("whitespace");
}

void WeatherUndergroundListener::startDocument() {
  //Serial.println("start document");
  this->index = -1;
}

void WeatherUndergroundListener::key(String key) {
  currentKey = String(key);

  // First Object key
  if (currentKey == "FCTTIME") {
     resetConditions();
     this->index += 1;
    this->conditions.index = this->index;
  }

}

void WeatherUndergroundListener::value(String value) {
  //Serial.println("value: " + value);

  if (currentParent == "temp" && currentKey == "metric") {
    conditions.currentTemp = value + " °C";
  }

  if (currentParent == "wspd" && currentKey == "metric") {
    conditions.windSpeed = value + " km/h";
  }

  if (currentParent == "wdir" && currentKey == "degrees") {
    conditions.windDir = value + " degrees";
  }

  if (currentKey == "icon") {
    conditions.weatherIcon = value;
  }

  if (currentKey == "condition") {
    conditions.weatherText = value;
  }

  if (currentKey == "humidity") {
    conditions.humidity = value + " %";
  }

  if (currentParent == "mslp" && currentKey == "metric") {
    conditions.pressure = value + " hPa";

    // HACK! Last Object key=>value, so all info has beed read and is ready
    this->isConditionsReady = true;
  }

  if (currentParent == "dewpoint" && currentKey == "metric") {
    conditions.dewPoint = value + " °C";
  }

  if (currentParent == "feelslike" && currentKey == "metric" ) {
    conditions.feelslike = value + " °C";
  }

  if (currentKey == "uvi") {
    conditions.UV = value;
  }

  if (currentKey == "hour") {
    conditions.hour = value;
  }

  if (currentKey == "mday") {
    conditions.monthDay = value;
  }
}

void WeatherUndergroundListener::endArray() {
  //Serial.println("end array. ");
}

void WeatherUndergroundListener::endObject() {
  currentParent = "";
  //Serial.println("end object. ");
}

void WeatherUndergroundListener::endDocument() {
  //Serial.println("end document. ");
}

void WeatherUndergroundListener::startArray() {
   //Serial.println("start array. ");
}

void WeatherUndergroundListener::startObject() {
  currentParent = currentKey;
  //Serial.println("start object. ");
}

void WeatherUndergroundListener::printConditions() {
  this->printConditions(&this->conditions);
}

void WeatherUndergroundListener::printConditions(WGConditions *conditions) {
  Serial.println("#---------------------------------------------------");
  Serial.print("Index: ");
  Serial.println(conditions->index);
  Serial.println("Current Temp: " + conditions->currentTemp);
  Serial.println("Wind Speed: " + conditions->windSpeed);
  Serial.println("Wind Dir: " + conditions->windDir);
  Serial.println("Weather Icon: " + conditions->weatherIcon);
  Serial.println("Weather Text: " + conditions->weatherText);
  Serial.println("Humidity: " + conditions->humidity);
  Serial.println("Pressure: " + conditions->pressure);
  Serial.println("Dew Point: " + conditions->dewPoint);
  Serial.println("Feels like: " + conditions->feelslike);
  Serial.println("UV: " + conditions->UV);
  Serial.println("Hour: " + conditions->hour);
  Serial.println("Month Day: " + conditions->monthDay);
}

void WeatherUndergroundListener::resetConditions() {
  this->conditions.index = 0;
  this->conditions.currentTemp = "";
  this->conditions.windSpeed = "";
  this->conditions.windDir = "";
  this->conditions.weatherIcon = "";
  this->conditions.weatherText = "";
  this->conditions.humidity = "";
  this->conditions.pressure = "";
  this->conditions.dewPoint = "";
  this->conditions.feelslike = "";
  this->conditions.UV = "";
  this->conditions.hour = "";
  this->conditions.monthDay = "";
}

#pragma once

#include "JsonListener.h"

typedef struct WGConditions {
  uint8_t index;
  String currentTemp;
  String windSpeed;
  String windDir;
  String weatherIcon;
  String weatherText;
  String humidity;
  String pressure;
  String dewPoint;
  String feelslike;
  String UV;
  String hour;
  String monthDay;
} WGConditions;


class WeatherUndergroundListener: public JsonListener {
  private:
    String currentParent;
    String currentKey;
    int8_t index;

    void resetConditions();

  public:
    WGConditions conditions;
    bool isConditionsReady = false;

    void printConditions();
    void printConditions(WGConditions *conditions);

    virtual void whitespace(char c);
  
    virtual void startDocument();

    virtual void key(String key);

    virtual void value(String value);

    virtual void endArray();

    virtual void endObject();

    virtual void endDocument();

    virtual void startArray();

    virtual void startObject();
};

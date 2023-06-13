#pragma once

#include "wled.h"

class InternalTemperatureUsermod : public Usermod
{

private:
  unsigned long loopInterval;
  unsigned long lastTime = 0;
  bool isEnabled;
  float temperature = 0;

  static const char _name[];
  static const char _enabled[];
  static const char _loopInterval[];

  // any private methods should go here (non-inline methosd should be defined out of class)
  void publishMqtt(const char *state, bool retain = false); // example for publishing MQTT message

public:
  void setup()
  {
  }

  void connected()
  {
  }

  void loop()
  {
    // if usermod is disabled or called during strip updating just exit
    // NOTE: on very long strips strip.isUpdating() may always return true so update accordingly
    if (!isEnabled || strip.isUpdating() || millis() - lastTime <= loopInterval)
      return;

    lastTime = millis();

#ifdef ESP8266 // ESP8266
    // does not seem possible
    temperature = -1;
#elif defined(CONFIG_IDF_TARGET_ESP32S2) // ESP32S2
    temperature = -1;
#else                                    // ESP32 ESP32S3 and ESP32C3
    temperature = roundf(temperatureRead() * 100) / 100;
#endif

#ifndef WLED_DISABLE_MQTT
    if (WLED_MQTT_CONNECTED)
    {
      char array[10];
      snprintf(array, sizeof(array), "%f", temperature);
      publishMqtt(array);
    }
#endif
  }

  void addToJsonInfo(JsonObject &root)
  {
    // if "u" object does not exist yet wee need to create it
    JsonObject user = root["u"];
    if (user.isNull())
      user = root.createNestedObject("u");

    JsonArray lightArr = user.createNestedArray(FPSTR(_name));
    lightArr.add(temperature);
    lightArr.add(F(" °C"));


    // if "sensor" object does not exist yet wee need to create it
    JsonObject sensor = root[F("sensor")];
    if (sensor.isNull())
      sensor = root.createNestedObject(F("sensor"));

    JsonArray temp = sensor.createNestedArray(FPSTR(_name));
    temp.add(temperature);
    temp.add(F("°C"));
  }

  void addToJsonState(JsonObject &root)
  {
  }
  void readFromJsonState(JsonObject &root)
  {
  }

  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject(FPSTR(_name));
    top[FPSTR(_enabled)] = isEnabled;
    top[FPSTR(_loopInterval)] = loopInterval;
  }

  bool readFromConfig(JsonObject &root)
  {
    JsonObject top = root[FPSTR(_name)];
    bool configComplete = !top.isNull();
    configComplete &= getJsonValue(top[FPSTR(_enabled)], isEnabled, false);
    configComplete &= getJsonValue(top[FPSTR(_loopInterval)], loopInterval, 1000);

    return configComplete;
  }

  void appendConfigData()
  {
  }

  void handleOverlayDraw()
  {
  }

  uint16_t getId()
  {
    return USERMOD_ID_INTERNAL_TEMPERATURE;
  }
};

const char InternalTemperatureUsermod::_name[] PROGMEM = "Internal Temperature";
const char InternalTemperatureUsermod::_enabled[] PROGMEM = "Enabled";
const char InternalTemperatureUsermod::_loopInterval[] PROGMEM = "Loop Interval";

void InternalTemperatureUsermod::publishMqtt(const char *state, bool retain)
{
#ifndef WLED_DISABLE_MQTT
  // Check if MQTT Connected, otherwise it will crash the 8266
  if (WLED_MQTT_CONNECTED)
  {
    char subuf[64];
    strcpy(subuf, mqttDeviceTopic);
    strcat_P(subuf, PSTR("/mcutemp"));
    mqtt->publish(subuf, 0, retain, state);
  }
#endif
}
#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>
#include <map>
#include "const.h"
#include "ArduinoJson.h"
#include "mqtt/mqtt.h"
#include "configs/config.h"

class Relay {
  private:
    int getRelayPin(int relay);
  public:
    void setup();
    std::map<int, int> getAllRelaysStatus();
    bool getRelayStatus(int relay);
    void updateAllRelays();
    void updateRelayStatus(int relay, int status);
    void turnOnRelay(int relay);
    void turnOffRelay(int relay);
    void turnOffAllRelays();
    void turnOnAllRelays();
    void handleAirConditioner(int hour, float temp);
    void handleLight(int hour);
    void handleHumidityDevice(int hour, int minute, float hum);
    void handleRelay(JsonDocument json);
    void parseAndSendRelayStatusMQTT();
    void turnOnAutoDevices();
    void setAutoMode(int relay, int mode);
    bool isRelayIdling();
};

#endif
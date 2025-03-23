#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "const.h"
#include "relay/relay.h"
#include "configs/config.h"

class MQTT {
  private:
    bool connect();
    void handleMqttEvent(byte *payload);
    static void callback(char *topic, byte *payload, uint length);
    void handleMqttConfig(byte *payload);
    void handleMqttGetConfig(byte *payload);

  public:
    void setup();
    bool reconnect();
    void handleMqtt();
    static void sendMessage(const char *channel, String message);
    static String createMqttMessage(String message, bool pushNotification, int isRestarted);
    static String createMqttPumpStatus(int type = 0);
    void publishConfig();
    void handleMqttGetInfo();
};

#endif
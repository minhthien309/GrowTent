#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>

class WifiConfig {
  public:
    static void setup();
    static void process();
};

#endif


#ifndef CONFIG_H
#define CONFIG_H

#include "const.h"
#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#define FORMAT_SPIFFS_IF_FAILED true

class Config {
  public:
    void setup();
    String readConfig(String file_name);
    void saveConfig(String filename, String value, bool is_append = false);
    JsonDocument getConfig();
    void updateConfig(String key, JsonDocument value, String filename);
  private:    
    void writeFile(fs::FS &fs, String filename, String message, bool is_append = false);
    String readFile(fs::FS &fs, String filename);
};

#endif
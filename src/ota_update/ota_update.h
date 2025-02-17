#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include <ArduinoOTA.h>
#include "const.h"

class OtaUpdate {
  public:
    void setup();
    void loop();
};

#endif
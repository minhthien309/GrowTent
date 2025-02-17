#ifndef HUM_TEMP_MIJIA_H
#define HUM_TEMP_MIJIA_H

#include <Arduino.h>
#include <BLEDevice.h>
#include "const.h"

struct HumTemp {
  HumTemp(float temp, float hum){
    temp = temp;
    hum = hum;
  };
  float hum;
  float temp;
};

class HumTempMijia: public BLEClientCallbacks {
  public:
    void onConnect(BLEClient *pclient);
    void onDisconnect(BLEClient *pclient);
    void getHumTemp();
    void connectSensor(BLEAddress htSensorAddress);
    void setup();
    HumTemp getHumTempObject();
  private:
    void registerNotification();
    static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
};

#endif
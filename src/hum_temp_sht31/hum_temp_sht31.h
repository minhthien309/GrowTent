#ifndef HUM_TEMP_SHT31_H
#define HUM_TEMP_SHT31_H

#include "const.h"
#include "Adafruit_SHT31.h"
#include <Arduino.h>

struct HumTemp {
  HumTemp(float temp, float hum){
    temp = temp;
    hum = hum;
  };
  float hum;
  float temp;
};

class HumTempSHT31 {
  public:
    HumTempSHT31(uint8_t address = I2CADDRESS);
    void getHumTemp();
    void setup();
    HumTemp getHumTempObject();
  private:
    uint8_t _address;
};
#endif
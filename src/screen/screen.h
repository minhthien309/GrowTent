#ifndef SCREEN_H
#define SCREEN_H

#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

class Screen {
  public:
    Screen();
    void setup();
    void sendText(String text, int x, int y, uint8_t* font);
};

#endif
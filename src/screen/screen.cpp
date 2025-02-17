#include "screen.h"

U8G2_ST7565_LM6059_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 5, /* dc=*/ 0, /* reset=*/ 4);

Screen::Screen() {

}

void Screen::setup() {
  u8g2.begin();
  u8g2.setContrast(32);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_4x6_tr);
}

void Screen::sendText(String text, int x, int y, uint8_t* font) {
  u8g2.clearBuffer();
  u8g2.setFont(font);
  
  u8g2.drawStr(x,y, text.c_str());

  u8g2.sendBuffer();
}
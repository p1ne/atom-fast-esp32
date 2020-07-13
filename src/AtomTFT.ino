#include "AtomTFT.h"

void drawDot(uint16_t color) {
  tft.setTextColor(0x39C4, TFT_BLACK);
  tft.drawString("*"+String(zabbixAttempts),196,96,4);
  tft.setTextColor(color, TFT_BLACK);
  tft.drawString("*"+String(zabbixAttempts),196,96,4);
}

void drawWiFi() {
  uint16_t color;
  switch (wifiStatus) {
    case WIFI_DISCONNECTED:
      color = TFT_RED;
      break;
    case WIFI_CONNECTING:
      color = TFT_YELLOW;
      break;
    case WIFI_CONNECTED:
      color = TFT_GREEN;
      break;
  }
  tft.setTextColor(0x39C4, TFT_BLACK);
  tft.drawString("W",196,66,4);
  tft.setTextColor(color, TFT_BLACK);
  tft.drawString("W",196,66,4);
}

void drawAtomScreen() {
  uint16_t doseColor = TFT_GREEN;

  if (currentAtomData.flags.doseExceed || currentAtomData.flags.dosePowerExceed) {
    doseColor = TFT_RED;
  }

  tft.setTextColor(0x39C4, TFT_BLACK);
  tft.drawString("8.8888",6,0,7);
  tft.setTextColor(doseColor, TFT_BLACK);
  tft.drawString(String(currentAtomData.dosePower, 4),6,0,7);

  tft.setTextColor(0x39C4, TFT_BLACK);
  tft.drawString("8.8888",6,48,7);
  tft.setTextColor(doseColor, TFT_BLACK);
  tft.drawString(String(currentAtomData.doseAccumulated, 4),6,48,7);

  tft.setTextColor(0x39C4, TFT_BLACK);
  tft.drawString("               ",6,96,4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(String(float(currentAtomData.impulses)/2.0, 1)+" CPS",6,96,4);

  tft.setTextColor(0x39C4, TFT_BLACK);
  tft.drawString("    ",180,0,4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(String(currentAtomData.charge)+"%",180,0,4);

  tft.setTextColor(0x39C4, TFT_BLACK);
  tft.drawString("    ",180,30,4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(String(currentAtomData.temperature)+"C",180,30,4);

  drawWiFi();
}

void tftSetup() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
}
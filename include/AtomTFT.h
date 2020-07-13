#ifndef __ATOM_TFT_H_
#define __ATOM_TFT_H_

#include <TFT_eSPI.h>
#include <SPI.h>

#include "AtomBLE.h"
#include "AtomData.h"
#include "AtomSettings.h"

TFT_eSPI tft = TFT_eSPI();

void drawDot(uint16_t color);
void drawWiFi(uint8_t wifiStatus);
void drawAtomScreen();
void tftSetup();

#endif // __ATOM_TFT_H_
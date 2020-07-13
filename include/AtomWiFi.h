#ifndef __ATOM_WIFI_H_
#define __ATOM_WIFI_H_

#include <WiFi.h>

#include "AtomBLE.h"
#include "AtomData.h"
#include "AtomTFT.h"
#include "AtomZabbix.h"
#include "AtomSettings.h"

#define WIFI_DISCONNECTED 0
#define WIFI_CONNECTING 1
#define WIFI_CONNECTED 2

WiFiServer server(80);
String header;

WiFiClient wifiClient = WiFiClient();

uint8_t wifiStatus = WIFI_DISCONNECTED;

void wifiAPSetup();
void wifiClientSetup();
void doWiFiServer();

#endif // __ATOM_WIFI_H_
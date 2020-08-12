#ifndef __ATOM_WIFI_H_
#define __ATOM_WIFI_H_

#include <WiFi.h>
#include "esp_wifi.h" // only for fixWifiPersistencyFlag()

#include "AtomBLE.h"
#include "AtomData.h"
#include "AtomZabbix.h"
#include "AtomSettings.h"

typedef enum {
  WIFI_DISCONNECTED = 0,
  WIFI_CONNECTING,
  WIFI_CONNECTED,
  WIFI_DISCONNECTING
} wifiState_t;

static wifiState_t wifiStatus = WIFI_DISCONNECTED;
static bool firstConnect = true;
static long stateSince = 0;

#define WIFI_TIMEOUT_ONLINE     2  // reconnect after this [s] offline time
#define WIFI_TIMEOUT_OFFLINE    20  // disconnect after this [s] online time
#define WIFI_TIMEOUT_CONNECTING 60  // cancel connecting after this [s] without success

WiFiClient wifiClient = WiFiClient();

void wifiClientSetup();


#endif // __ATOM_WIFI_H_
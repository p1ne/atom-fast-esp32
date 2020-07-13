#include "AtomData.h"
#include "AtomTFT.h"
#include "AtomZabbix.h"
#include "AtomBLE.h"
#include "AtomWiFi.h"


void setup() {
  Serial.begin(115200);
  tftSetup();
  zabbixSetup();
  bleSetup();
  xTaskCreate(doZabbix, "Zabbix", 32767, NULL, 5, NULL);	
}

void loop() {
	doBLE();
}

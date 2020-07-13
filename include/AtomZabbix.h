#ifndef __ATOM_ZABBIX_H_
#define __ATOM_ZABBIX_H_

#include <ESP32ZabbixSender.h>

#include "AtomBLE.h"
#include "AtomData.h"
#include "AtomWiFi.h"
#include "AtomSettings.h"

ESP32ZabbixSender zSender;

QueueHandle_t zabbixQueue;

#define ZABBIX_MAX_RETRIES 10

uint8_t zabbixAttempts = 0;

void zabbixSetup();
void zabbixSend(float value);

#endif // __ATOM_ZABBIX_H_

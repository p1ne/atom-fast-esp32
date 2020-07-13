#ifndef __ATOM_BLE_H_
#define __ATOM_BLE_H_

#include "BLEDevice.h"

#include "AtomData.h"
#include "AtomZabbix.h"
#include "AtomSettings.h"

#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

#undef CLASSIC_BT_ENABLED

static BLEUUID serviceUUID("63462a4a-c28c-4ffd-87a4-2d23a1c72581");
static BLEUUID    charUUID("70bc767e-7a1a-4304-81ed-14b9af54f7bd");

static boolean doBLEConnect = false;
static boolean bleConnected = false;
static boolean doBLEScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
static BLEClient*  pClient;

bool connectToServer();
void bleSetup();
void doBLE();
#endif // __ATOM_BLE_H_
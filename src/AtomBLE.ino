#include "AtomBLE.h"

static void notifyCallback(
  NimBLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  
  Serial.print("[BLE] Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");
  for (int i=0;i<length;i++) {
    Serial.print(pData[i], HEX);
  }
  Serial.println();

  if (length == 13) {
    parseAtomData(currentAtomData, pData);
  }
  Serial.print("[BLE] Sending dose power value to Zabbix: ");
  Serial.println(currentAtomData.dosePower);
  xQueueSend(zabbixQueue, &currentAtomData.dosePower, 0);
}

void scanEndedCallback(NimBLEScanResults results){
    Serial.println("Scan Ended");
}

class ClientCallbacks : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient* pClient) {
    //pClient->updateConnParams(120,120,0,60);
    pClient->updateConnParams(60,60,0,40);
  };

  void onDisconnect(NimBLEClient* pClient) {
    Serial.println("[BLE] onDisconnect");
    bleStatus = BLE_DISCONNECTED;
    float val = -1.0;
    xQueueSend(zabbixQueue, &val, 0);
    NimBLEDevice::getScan()->start(scanTime, scanEndedCallback);
  };

  bool onConnParamsUpdateRequest(NimBLEClient* pClient, const ble_gap_upd_params* params) {
      if(params->itvl_min < 24) { /** 1.25ms units */
          return false;
      } else if(params->itvl_max > 40) { /** 1.25ms units */
          return false;
      } else if(params->latency > 2) { /** Number of intervals allowed to skip */
          return false;
      } else if(params->supervision_timeout > 100) { /** 10ms units */
          return false;
      }

      return true;
  };
};

static ClientCallbacks clientCallback;

void connectToBLEServer() {
    NimBLEClient* pClient = nullptr;
    
    if(NimBLEDevice::getClientListSize()) {
        pClient = NimBLEDevice::getClientByPeerAddress(bleDevice->getAddress());
        if(pClient){
            if(!pClient->connect(bleDevice, false)) {
                Serial.println("Reconnect failed");
                bleStatus = BLE_DISCONNECTED;
                return;
            }
            Serial.println("Reconnected client");
        } else {
            pClient = NimBLEDevice::getDisconnectedClient();
        }
    }
    
    if(!pClient) {
        if(NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS) {
            Serial.println("Max clients reached - no more connections available");
            bleStatus = BLE_DISCONNECTED;
            return;
        }
        
        pClient = NimBLEDevice::createClient();
        
        Serial.println("New client created");
    
        pClient->setClientCallbacks(&clientCallback, false);
        pClient->setConnectionParams(18,18,0,60);
        pClient->setConnectTimeout(30);

        if (!pClient->connect(bleDevice)) {
            NimBLEDevice::deleteClient(pClient);
            Serial.println("Failed to connect, deleted client");
            bleStatus = BLE_DISCONNECTED;
            return;
        }
    }         
    
    if(!pClient->isConnected()) {
        if (!pClient->connect(bleDevice)) {
            Serial.println("Failed to connect");
            bleStatus = BLE_DISCONNECTED;
            return;
        }
    }
    
    Serial.print("Connected to: ");
    Serial.println(pClient->getPeerAddress().toString().c_str());
    Serial.print("RSSI: ");
    Serial.println(pClient->getRssi());
    
    NimBLERemoteService* pSvc = nullptr;
    NimBLERemoteCharacteristic* pChr = nullptr;
    
    pSvc = pClient->getService(serviceUUID);
    if(pSvc) {
        pChr = pSvc->getCharacteristic(charUUID);
    }

    if(pChr) {
        if(pChr->canRead()) {
            Serial.print(pChr->getUUID().toString().c_str());
            Serial.print(" Value: ");
            Serial.println(pChr->readValue().c_str());
        }
        
        if(pChr->canNotify()) {
            if(!pChr->subscribe(true, notifyCallback)) {
                pClient->disconnect();
                bleStatus = BLE_DISCONNECTED;
                return;
            } else {
              bleStatus = BLE_CONNECTED;
              float val = -2.0;
              xQueueSend(zabbixQueue, &val, 0);
            }
        } else {
          bleStatus = BLE_DISCONNECTED;
        }
    } else {
        Serial.println("Service not found.");
        bleStatus = BLE_DISCONNECTED;
    }
}

class AtomAdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
  void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
    Serial.print("Advertised Device found: ");
    Serial.println(advertisedDevice->toString().c_str());
    if ((advertisedDevice->getAddress().toString() == BSSID) && advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(serviceUUID)) {
      Serial.println("Found Our Service");
      NimBLEDevice::getScan()->stop();
      bleDevice = advertisedDevice;
      bleStatus = BLE_CONNECTING;
    }
  };
};

void bleSetup() {
  Serial.println("[BLE] Starting BLE Client");
  NimBLEDevice::init("");
  //NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */

  NimBLEScan* pNimBLEScan = NimBLEDevice::getScan();
  pNimBLEScan->setAdvertisedDeviceCallbacks(new AtomAdvertisedDeviceCallbacks());
  pNimBLEScan->setInterval(450);
  pNimBLEScan->setWindow(200);
  pNimBLEScan->setActiveScan(true);
  bleStatus = BLE_SCANNING;
  pNimBLEScan->start(scanTime, scanEndedCallback);
}

void doBLE() {
  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed=1;
  TIMERG0.wdt_wprotect=0;

while (true) {
    while (bleStatus == BLE_CONNECTING) {
      vTaskDelay(100 / portTICK_RATE_MS);
      connectToBLEServer();
      if (bleStatus == BLE_CONNECTED) {
        Serial.println("[BLE] Connected to server");
      } else {
        Serial.println("[BLE] Failed to connect to server");
      }
    }

    if (bleStatus == BLE_DISCONNECTED) {
      bleStatus = BLE_SCANNING;
      NimBLEDevice::getScan()->start(scanTime,scanEndedCallback);
    }
    yield();
  }
}

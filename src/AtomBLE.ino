#include "AtomBLE.h"

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
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

  drawAtomScreen();
  xQueueSend(zabbixQueue, &currentAtomData.dosePower, 0);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pClient) {
  }

  void onDisconnect(BLEClient* pClient) {
    Serial.println("[BLE] onDisconnect");
    bleConnected = false;
    doBLEConnect = true;
  }
};

void createBLEClient() {
    Serial.print("[BLE] Forming a connection to ");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("Atom connecting...",6,0,4);

    Serial.println(myDevice->getAddress().toString().c_str());
    
    pClient  = BLEDevice::createClient();
    Serial.println("[BLE] - Created client");

    pClient->setClientCallbacks(new MyClientCallback());
}

bool connectToBLEServer() {
    if (pClient->isDisconnected() && pClient->connect(myDevice)) {
      Serial.println("[BLE] - Connected to server");
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.drawString("Atom connected",6,0,4);

      BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
      if (pRemoteService == nullptr) {
        Serial.print("[BLE] Failed to find our service UUID: ");
        Serial.println(serviceUUID.toString().c_str());
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("Atom failed (service)",6,0,4);
        pClient->disconnect();
        return false;
      }
      Serial.println("[BLE] - Found our service");

      pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
      if (pRemoteCharacteristic == nullptr) {
        Serial.print("[BLE] Failed to find our characteristic UUID: ");
        Serial.println(charUUID.toString().c_str());
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("Atom failed (characteristic)",6,0,4);
        pClient->disconnect();
        return false;
      }
      Serial.println("[BLE] - Found our characteristic");

      if(pRemoteCharacteristic->canRead()) {
        std::string value = pRemoteCharacteristic->readValue();
        Serial.print("[BLE] The characteristic value was: ");
        for (int i=0;i<value.length();i++) {
          Serial.print(value[i], HEX);
        }
        Serial.println();
      }

      if(pRemoteCharacteristic->canNotify())
        pRemoteCharacteristic->registerForNotify(notifyCallback);

      bleConnected = true;
    } else {
      bleConnected = false;
    }
    return bleConnected;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("[BLE] BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    if ((advertisedDevice.getAddress().toString() == BSSID) && advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.drawString("Atom found",6,0,4);
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doBLEConnect = true;
      doBLEScan = false;
    }
  }
};

void bleSetup() {
  Serial.println("[BLE] Starting BLE Client");
  BLEDevice::init("");
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("Searching for Atom Fast...",6,0,4);

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(450);
  pBLEScan->setWindow(200);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30, false);

  createBLEClient();
}

void doBLE() {
  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed=1;
  TIMERG0.wdt_wprotect=0;

  while (true) {
    if (doBLEConnect == true) {
      if (connectToBLEServer()) {
        Serial.println("[BLE] Connected to server");
        if (wifiStatus == WIFI_DISCONNECTED) {
          wifiClientSetup();
        }
      } else {
        Serial.println("[BLE] Failed to connect to server");
      }
      doBLEConnect = false;
    }

    if(doBLEScan){
      BLEDevice::getScan()->start(0);
    }
    //vTaskDelay(100 / portTICK_RATE_MS);
    
    //unsigned long thedelay;
    //thedelay = micros() + 100;
    //while (micros() < thedelay) {
    //}
    yield();
    //delay(100);
  }
}

#include "BLEDevice.h"
#include <TFT_eSPI.h>
#include <SPI.h>

////// TFT
TFT_eSPI tft = TFT_eSPI();

///// BLE
static BLEUUID serviceUUID("63462a4a-c28c-4ffd-87a4-2d23a1c72581");
static BLEUUID    charUUID("70bc767e-7a1a-4304-81ed-14b9af54f7bd");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

struct AtomDataFlags {
  uint8_t doseExceed: 1;
  uint8_t dosePowerExceed: 1;
  uint8_t countSpeedInc: 1;
  uint8_t spare: 1;
  uint8_t currentExceed: 1;
  uint8_t detectorOverload: 1;
  uint8_t chargerConnected: 1;
  uint8_t emergencyShutdown: 1;
};

struct AtomData {
    AtomDataFlags flags;
    float doseAccumulated;
    float dosePower;
    uint16_t impulses;
    uint8_t charge;
    int8_t temperature;
};

AtomData currentAtomData;

float getFloat(uint8_t* pData, uint8_t startIdx) {
  union {
    float val;
    uint8_t b[4];
  } float_bytes_u;

  for (uint8_t i=0;i<4;i++) {
    float_bytes_u.b[i] = pData[i+startIdx];
  }

  return float_bytes_u.val;
}

uint16_t getWord(uint8_t* pData, uint8_t startIdx) {
  union {
    uint16_t val;
    uint8_t b[2];
  } uint16_bytes_u;

  uint16_bytes_u.b[0] = pData[startIdx];
  uint16_bytes_u.b[1] = pData[startIdx+1];

  return uint16_bytes_u.val;
}

AtomDataFlags getFlags(uint8_t* pData) {
  union {
    AtomDataFlags val;
    uint8_t b;
  } flags_bytes_u;

  flags_bytes_u.b = pData[0];

  return flags_bytes_u.val;
}

void parseAtomData(AtomData& atomData, uint8_t* pData) {
  atomData.flags = getFlags(pData);
  atomData.doseAccumulated = getFloat(pData, 1);
  atomData.dosePower = getFloat(pData, 5);
  atomData.impulses = getWord(pData, 9);
  atomData.charge = pData[11];
  atomData.temperature = pData[12];
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
  tft.drawString(String(currentAtomData.impulses, 4)+" imp/s",6,96,4);

  tft.setTextColor(0x39C4, TFT_BLACK);
  tft.drawString("    ",180,0,4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(String(currentAtomData.charge)+"%",180,0,4);

  tft.setTextColor(0x39C4, TFT_BLACK);
  tft.drawString("    ",180,30,4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(String(currentAtomData.temperature)+"C",180,30,4);
}

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
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pClient) {
  }

  void onDisconnect(BLEClient* pClient) {
    connected = false;
    Serial.println("[BLE] onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("[BLE] Forming a connection to ");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("Connecting...",6,0,4);

    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println("[BLE] - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    pClient->connect(myDevice);
    Serial.println("[BLE] - Connected to server");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("Connected",6,0,4);

    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("[BLE] Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("Failed (service)",6,0,4);
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
      tft.drawString("Failed (characteristic)",6,0,4);
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

    connected = true;
    return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("[BLE] BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
    }
  }
};

void tftSetup() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
}

void bleSetup() {
  Serial.println("[BLE] Starting BLE Client");
  BLEDevice::init("");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30, false);
}

void doBLE() {
  while (true) {
    if (doConnect == true) {
      if (connectToServer()) {
        Serial.println("[BLE] Connected to server");
      } else {
        Serial.println("[BLE] Failed to connect to server");
      }
      doConnect = false;
    }

    if(doScan){
      BLEDevice::getScan()->start(0);
    }
    delay(1000);
  }
}

void setup() {
  Serial.begin(115200);
  tftSetup();
  bleSetup();
}

void loop() {
  doBLE();  
}

/*
 *
 * Created Jun 2, 2022
 * Author: HoaLe
 */
#include <Arduino.h>
#include <NimBLEDevice.h>

#define BOMB_NO 6
const uint16_t SERVICE_UUID = 22677;

const uint8_t GPIO_PIN_LIST[BOMB_NO] = {
  GPIO_NUM_12, // 12
  GPIO_NUM_14, // 14
  GPIO_NUM_27, // 27
  GPIO_NUM_25, // 26
  GPIO_NUM_26, // 25
  GPIO_NUM_32 // 32
};

const std::string CHAR_UUID_LIST[BOMB_NO] = {
  "00000000-0000-0000-0000-000000000001", 
  "00000000-0000-0000-0000-000000000002", 
  "00000000-0000-0000-0000-000000000003", 
  "00000000-0000-0000-0000-000000000004", 
  "00000000-0000-0000-0000-000000000005", 
  "00000000-0000-0000-0000-000000000006", 
};

static BLEServer* pServer;

class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        Serial.println("Client connected");
        Serial.println("Multi-connect support: start advertising");
        NimBLEDevice::startAdvertising();
    };
    void onDisconnect(BLEServer* pServer) {
        Serial.println("Client disconnected - start advertising");
        NimBLEDevice::startAdvertising();
    };
};

void setup() {
  // Serial
  Serial.begin(115200);


  // BLE
  NimBLEDevice::init("BigBang");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);

  NimBLEDevice::setSecurityAuth(true, true, true);
  NimBLEDevice::setSecurityPasskey(123456);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());
  NimBLEService *pService = pServer->createService(NimBLEUUID(SERVICE_UUID));

  for (size_t i = 0; i < BOMB_NO; i++)
  {
    NimBLECharacteristic *pCharacteristic = pService->createCharacteristic(
      BLEUUID(CHAR_UUID_LIST[i]),
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE
    );
  }

  pService->start();

  // GPIO
  for (size_t i = 0; i < BOMB_NO; i++)
  {
    pinMode(GPIO_PIN_LIST[i], OUTPUT); // output
    digitalWrite(GPIO_PIN_LIST[i], LOW);
    NimBLECharacteristic *pCharacteristic = pService->getCharacteristic(NimBLEUUID(CHAR_UUID_LIST[i]));
    uint8_t gpioState = digitalRead(GPIO_PIN_LIST[i]);
    pCharacteristic->setValue(&gpioState, 1);
  }
  
  NimBLEAdvertising* pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMaxPreferred(0x12);
  pAdvertising->start();
  Serial.println("Advertising Started");
}

void loop() {
  if(pServer->getConnectedCount()) {
    NimBLEService* pService = pServer->getServiceByUUID(SERVICE_UUID);
    if(pService) {
      for (size_t i = 0; i < BOMB_NO; i++)
      {
        NimBLECharacteristic *pCharacteristic = pService->getCharacteristic(
          NimBLEUUID(CHAR_UUID_LIST[i])
        );
        std::string rxValue = pCharacteristic->getValue();
        Serial.print(rxValue.c_str());
        
        if (rxValue.length() > 0 && rxValue.at(0) == 1) { 
          digitalWrite(GPIO_PIN_LIST[i], HIGH);
        } else {
          digitalWrite(GPIO_PIN_LIST[i], LOW);
        }
        pCharacteristic->notify();
        Serial.print(",");
      }
      Serial.println("");
    }
  }

  delay(2000);
}
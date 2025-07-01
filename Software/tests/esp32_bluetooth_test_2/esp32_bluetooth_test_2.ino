#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* ble_server = NULL;
BLECharacteristic* ble_sensor = NULL;
BLECharacteristic* ble_proximity = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

/*
A-104.9903,39.7392+23.5+65.2+1013.25A
*/

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID        "19b10000-e8f2-537e-4f6c-d104768a1214"
#define SENSOR_CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"
#define PROXIMITY_CHARACTERISTIC_UUID "19b10002-e8f2-537e-4f6c-d104768a1214"

class ble_server_callbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* ble_server) { deviceConnected = true; };
  void onDisconnect(BLEServer* ble_server) { deviceConnected = false; }
};

class ble_proximity_callbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* ble_proximity) {
    String recv_value = ble_proximity->getValue();
    if (recv_value.length() > 0) {
      Serial.print("Characteristic (proximity) event, written: ");
      Serial.println(recv_value[0], HEX);
    }
  }
};

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("opaque - Personal Kit");
  ble_server = BLEDevice::createServer();
  ble_server->setCallbacks(new ble_server_callbacks());
  

  // Create the BLE Service
  BLEService* ble_service = ble_server->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  ble_sensor = ble_service->createCharacteristic(
                      SENSOR_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // Create the ON button Characteristic
  ble_proximity = ble_service->createCharacteristic(
                      PROXIMITY_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  ble_proximity->setCallbacks(new ble_proximity_callbacks());

  ble_sensor->addDescriptor(new BLE2902());
  ble_proximity->addDescriptor(new BLE2902());

  ble_service->start();

  // Start advertising
  BLEAdvertising* ble_advertising = BLEDevice::getAdvertising();
  ble_advertising->addServiceUUID(SERVICE_UUID);
  ble_advertising->setScanResponse(false);
  ble_advertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  String instring = "";
  if (Serial.available()) {instring = Serial.readString();
  if (deviceConnected) {
    ble_sensor->setValue(instring.c_str());
    ble_sensor->notify();
    Serial.print("New value notified: ");
    Serial.println(instring);
    delay(3000); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
  }}
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    Serial.println("Device disconnected.");
    delay(500); // give the bluetooth stack the chance to get things ready
    ble_server->startAdvertising(); // restart advertising
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
    Serial.println("Device Connected");
  }
}
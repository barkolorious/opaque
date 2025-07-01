#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <esp_now.h>
#include <WiFi.h>

BLEServer* ble_server = NULL;
BLECharacteristic* ble_sensor = NULL;
bool ble_connected = false;
bool old_ble_connected = false;
#define SERVICE_UUID        "19b10000-e8f2-537e-4f6c-d104768a1214"
#define SENSOR_CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"
class ble_server_callbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* ble_server) { 
    Serial.println("BLE Device connected");
    ble_connected = true; 
  };
  void onDisconnect(BLEServer* ble_server) { 
    Serial.println("BLE Device disconnected");
    ble_connected = false; 
    BLEDevice::startAdvertising();
  };
};

uint8_t broadcastAddress[] = {0x5C, 0xCF, 0x7F, 0x16, 0x75, 0x9A};
typedef struct struct_message { bool alert; } struct_message;
struct_message myData;
esp_now_peer_info_t peerInfo;
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

int mode = 0;
int old_mode = 0;
bool ilk_defa = true;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

}

void espnow_init () {
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
    
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void espnow_deinit () {
  Serial.println("Shutting down ESP-NOW...");

  esp_now_unregister_send_cb();
  
  esp_now_peer_info_t peer;
  
  if (esp_now_is_peer_exist(broadcastAddress)) {
    esp_err_t result = esp_now_del_peer(broadcastAddress);
    if (result == ESP_OK) {
      Serial.println("Peer removed successfully");
    } else {
      Serial.println("Failed to remove peer");
    }
  }
  
  esp_err_t result = esp_now_deinit();
  if (result == ESP_OK) {
    Serial.println("ESP-NOW deinitialized successfully");
  } else {
    Serial.println("Failed to deinitialize ESP-NOW");
  }
  
  WiFi.mode(WIFI_OFF);
  
  Serial.println("ESP-NOW shutdown complete");
  delay(500); // Give time for cleanup
}

void ble_init () {
  Serial.println("A");
  BLEDevice::init("opaque - Personal Kit");
  Serial.println("A");
  ble_server = BLEDevice::createServer();
  Serial.println("A");
  ble_server->setCallbacks(new ble_server_callbacks());
  Serial.println("A");

  BLEService* ble_service = ble_server->createService(SERVICE_UUID);
  ble_sensor = ble_service->createCharacteristic(SENSOR_CHARACTERISTIC_UUID, 0b00010111);
  ble_sensor->addDescriptor(new BLE2902());
  Serial.println("A");
  ble_service->start();
  Serial.println("A");
  
  BLEAdvertising* ble_advertising = BLEDevice::getAdvertising();
  ble_advertising->addServiceUUID(SERVICE_UUID);
  ble_advertising->setScanResponse(false);
  ble_advertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("A");
  Serial.println("Waiting a client connection to notify...");
  ilk_defa = false;
}

void ble_reinit () {
  Serial.println("A");
  if (esp_bt_controller_get_status() != ESP_BT_CONTROLLER_STATUS_ENABLED) {
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    
    esp_err_t ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
      Serial.println("Controller init failed");
      return;
    }
    
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
      Serial.println("Controller enable failed");
      return;
    }
  }
  Serial.println("A");
  BLEDevice::init("opaque - Personal Kit");
  Serial.println("A");
  ble_server = BLEDevice::createServer();
  Serial.println("A");
  ble_server->setCallbacks(new ble_server_callbacks());
  Serial.println("A");

  BLEService* ble_service = ble_server->createService(SERVICE_UUID);
  ble_sensor = ble_service->createCharacteristic(SENSOR_CHARACTERISTIC_UUID, 0b00010111);
  ble_sensor->addDescriptor(new BLE2902());
  Serial.println("A");
  ble_service->start();
  Serial.println("A");
  
  BLEAdvertising* ble_advertising = BLEDevice::getAdvertising();
  ble_advertising->addServiceUUID(SERVICE_UUID);
  ble_advertising->setScanResponse(false);
  ble_advertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("A");
  Serial.println("Waiting a client connection to notify...");
}

void ble_deinit () {
  Serial.println("Shutting down BLE...");
  
  // Stop advertising first
  BLEDevice::stopAdvertising();
  delay(100);
  
  // Disconnect any connected clients
  if (ble_server != nullptr && ble_server->getConnectedCount() > 0) {
    std::map<uint16_t, conn_status_t> connections = ble_server->getPeerDevices(true);
    for (auto const& entry : connections) {
      ble_server->disconnect(entry.first);
    }
    delay(500); // Give more time for disconnection
  }
  
  // Clean up services and characteristics
  if (ble_server != nullptr) {
    // Don't remove services manually - deinit handles this
  }
  
  // Deinitialize BLE completely
  BLEDevice::deinit(true); // true = release all memory
  
  esp_bt_controller_disable();
  esp_bt_controller_deinit();

  ble_server = nullptr;
  ble_sensor = nullptr;
  
  Serial.println("BLE shutdown complete");
  delay(1000); // Longer delay for complete cleanup
}


// A-104.9903,49.7392+23.5+65.2+1013.25A

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    if (Serial.peek() == 'A' && mode == 1) {
      Serial.println("BLE");
      ble_update();
    } else if ((Serial.peek() == 'B' || Serial.peek() == 'C') && mode == 2) {
      Serial.println("ESP-NOW");
      espnow_update();
    } else if (Serial.peek() == '0' || Serial.peek() == '1' || Serial.peek() == '2') {
      mode = Serial.read() - '0'; // 0 -> Nothing, 1 -> BLE, 2 -> ESP-NOW
      if (mode == 1 && old_mode == 0) {
        Serial.println("from Nothing to BLE");
        if (ilk_defa) ble_init();
        else ble_reinit();
      } else if (mode == 1 && old_mode == 2) {
        Serial.println("from ESP-NOW to BLE");
        espnow_deinit();
        if (ilk_defa) ble_init();
        else ble_reinit();
      } else if (mode == 2 && old_mode == 0) {
        Serial.println("from Nothing to ESP-NOW");
        espnow_init();
      } else if (mode == 2 && old_mode == 1) {
        Serial.println("from BLE to ESP-NOW");
        ble_deinit();
        espnow_init();
      } else if (mode == 0 && old_mode == 1) {
        Serial.println("from BLE to Nothing");
        ble_deinit();
      } else if (mode == 0 && old_mode == 2) {
        Serial.println("from ESP-NOW to Nothing");
        espnow_deinit();
      }
      old_mode = mode;
      while (Serial.available()) Serial.read();
    } else {
      while (Serial.available()) Serial.read();
    }
  }
  Serial.println(mode);

  // disconnecting
  if (mode == 1) {
    if (!ble_connected && old_ble_connected) {
      Serial.println("Device disconnected.");
      delay(500); // give the bluetooth stack the chance to get things ready
      ble_server->startAdvertising(); // restart advertising
      Serial.println("Start advertising");
      old_ble_connected = ble_connected;
    }
    // connecting
    if (ble_connected && !old_ble_connected) {
      // do stuff here on connecting
      old_ble_connected = ble_connected;
      Serial.println("Device Connected");
    }
  }

  delay(100);
}

void ble_update () {
  String instring = "";
  if (Serial.available()) {
    instring = Serial.readString();
    if (ble_connected) {
      ble_sensor->setValue(instring.c_str());
      ble_sensor->notify();
      Serial.print("New value notified: ");
      Serial.println(instring);
    }
  }
}

void espnow_update () {
  myData.alert = Serial.read() - 'B';
  while (Serial.available()) Serial.read();

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
}
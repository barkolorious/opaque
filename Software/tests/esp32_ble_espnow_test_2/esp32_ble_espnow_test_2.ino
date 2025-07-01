#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_bt.h>

// BLE variables
BLEServer* ble_server = NULL;
BLECharacteristic* ble_sensor = NULL;
bool ble_connected = false;
bool old_ble_connected = false;

// UUIDs
#define SERVICE_UUID        "19b10000-e8f2-537e-4f6c-d104768a1214"
#define SENSOR_CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"

// ESP-NOW variables
uint8_t broadcastAddress[] = {0x5C, 0xCF, 0x7F, 0x16, 0x75, 0x9A};
typedef struct struct_message { 
  bool alert; 
} struct_message;
struct_message myData;
esp_now_peer_info_t peerInfo;

// Mode control
enum Mode { MODE_OFF = 0, MODE_BLE = 1, MODE_ESPNOW = 2 };
Mode current_mode = MODE_OFF;
Mode previous_mode = MODE_OFF;
bool first_init = true;

// BLE Server Callbacks
class ble_server_cb: public BLEServerCallbacks {
  void onConnect(BLEServer* server) { 
    Serial.println("BLE Device connected");
    ble_connected = true; 
  }
  
  void onDisconnect(BLEServer* server) { 
    Serial.println("BLE Device disconnected");
    ble_connected = false; 
    // Don't restart advertising here - let the main loop handle it
  }
};

// ESP-NOW callback
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("ESP-NOW Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failed");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 BLE/ESP-NOW Switcher");
  Serial.println("Commands: 0=Off, 1=BLE, 2=ESP-NOW");
  Serial.println("BLE: Send 'A' + data, ESP-NOW: Send 'B' or 'C'");
}

bool initESPNOW() {
  // Set WiFi to station mode
  WiFi.mode(WIFI_STA);
  delay(100);
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return false;
  }
  
  // Register send callback
  esp_now_register_send_cb(OnDataSent);
  
  // Setup peer info
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add ESP-NOW peer");
    return false;
  }
  
  Serial.println("ESP-NOW initialized successfully");
  return true;
}

void deinitESPNOW() {
  Serial.println("Shutting down ESP-NOW...");
  
  // Unregister callback
  esp_now_unregister_send_cb();
  
  // Remove peer if exists
  if (esp_now_is_peer_exist(broadcastAddress)) {
    esp_err_t result = esp_now_del_peer(broadcastAddress);
    if (result == ESP_OK) {
      Serial.println("ESP-NOW peer removed");
    } else {
      Serial.println("Failed to remove ESP-NOW peer");
    }
  }
  
  // Deinitialize ESP-NOW
  esp_err_t result = esp_now_deinit();
  if (result == ESP_OK) {
    Serial.println("ESP-NOW deinitialized");
  } else {
    Serial.println("Failed to deinitialize ESP-NOW");
  }
  
  // Turn off WiFi completely and wait
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(1000); // Longer delay to ensure WiFi is completely off
}

bool initBLE() {
  Serial.println("Initializing BLE...");
  
  // Ensure WiFi is completely off before starting BLE
  WiFi.mode(WIFI_OFF);
  delay(500);
  
  // Initialize and enable Bluetooth controller if needed
  esp_bt_controller_status_t bt_status = esp_bt_controller_get_status();
  Serial.print("Current BT controller status: ");
  Serial.println(bt_status);
  
  if (bt_status == ESP_BT_CONTROLLER_STATUS_IDLE) {
    Serial.println("Initializing Bluetooth controller...");
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    
    // Reduce memory usage for better compatibility
    bt_cfg.mode = ESP_BT_MODE_BLE;
    bt_cfg.ble_max_conn = 1;
    bt_cfg.bt_max_acl_conn = 0;
    bt_cfg.bt_max_sync_conn = 0;
    
    esp_err_t ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
      Serial.print("Failed to init Bluetooth controller: 0x");
      Serial.println(ret, HEX);
      return false;
    }
    Serial.println("Bluetooth controller initialized");
  }
  
  if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_INITED) {
    Serial.println("Enabling Bluetooth controller...");
    esp_err_t ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
      Serial.print("Failed to enable Bluetooth controller: 0x");
      Serial.println(ret, HEX);
      
      // Try to recover by deinitializing and starting fresh
      Serial.println("Attempting recovery...");
      esp_bt_controller_deinit();
      delay(1000);
      
      esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
      bt_cfg.mode = ESP_BT_MODE_BLE;
      
      ret = esp_bt_controller_init(&bt_cfg);
      if (ret == ESP_OK) {
        ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
      }
      
      if (ret != ESP_OK) {
        Serial.println("Recovery failed");
        return false;
      }
    }
    Serial.println("Bluetooth controller enabled");
  }
  
  // Initialize BLE device
  BLEDevice::init("ESP32-BLE-ESPNOW");
  
  // Create BLE Server
  ble_server = BLEDevice::createServer();
  if (!ble_server) {
    Serial.println("Failed to create BLE server");
    return false;
  }
  
  ble_server->setCallbacks(new ble_server_cb());
  
  // Create BLE Service
  BLEService* service = ble_server->createService(SERVICE_UUID);
  if (!service) {
    Serial.println("Failed to create BLE service");
    return false;
  }
  
  // Create BLE Characteristic
  ble_sensor = service->createCharacteristic(
    SENSOR_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_INDICATE
  );
  
  if (!ble_sensor) {
    Serial.println("Failed to create BLE characteristic");
    return false;
  }
  
  ble_sensor->addDescriptor(new BLE2902());
  
  // Start service
  service->start();
  
  // Start advertising
  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(false);
  advertising->setMinPreferred(0x0);
  
  BLEDevice::startAdvertising();
  
  Serial.println("BLE initialized and advertising");
  return true;
}

void deinitBLE() {
  Serial.println("Shutting down BLE...");
  
  // Stop advertising
  BLEDevice::stopAdvertising();
  delay(200);
  
  // Disconnect clients gracefully
  if (ble_server && ble_server->getConnectedCount() > 0) {
    Serial.println("Disconnecting BLE clients...");
    // Force disconnect by stopping the server
    delay(500);
  }
  
  // Clean up variables
  ble_connected = false;
  old_ble_connected = false;
  ble_server = nullptr;
  ble_sensor = nullptr;
  
  // Deinitialize BLE completely
  BLEDevice::deinit(true);
  delay(500);
  
  // Disable and deinit Bluetooth controller with better error handling
  esp_bt_controller_status_t status = esp_bt_controller_get_status();
  
  if (status == ESP_BT_CONTROLLER_STATUS_ENABLED) {
    Serial.println("Disabling Bluetooth controller...");
    esp_err_t ret = esp_bt_controller_disable();
    if (ret == ESP_OK) {
      Serial.println("Bluetooth controller disabled");
    } else {
      Serial.print("Failed to disable BT controller: 0x");
      Serial.println(ret, HEX);
    }
    delay(500);
  }
  
  status = esp_bt_controller_get_status();
  if (status == ESP_BT_CONTROLLER_STATUS_INITED) {
    Serial.println("Deinitializing Bluetooth controller...");
    esp_err_t ret = esp_bt_controller_deinit();
    if (ret == ESP_OK) {
      Serial.println("Bluetooth controller deinitialized");
    } else {
      Serial.print("Failed to deinit BT controller: 0x");
      Serial.println(ret, HEX);
    }
  }
  
  Serial.println("BLE shutdown complete");
  delay(1000);
}

bool switchToMode(Mode new_mode) {
  if (new_mode == current_mode) {
    Serial.println("Already in requested mode");
    return true;
  }
  
  previous_mode = current_mode;
  
  // Shutdown current mode
  switch (current_mode) {
    case MODE_BLE:
      deinitBLE();
      break;
    case MODE_ESPNOW:
      deinitESPNOW();
      break;
    case MODE_OFF:
      break;
  }
  
  delay(500); // Give time for cleanup
  
  // Initialize new mode
  bool success = true;
  switch (new_mode) {
    case MODE_BLE:
      success = initBLE();
      if (success) {
        Serial.println("Switched to BLE mode");
      }
      break;
    case MODE_ESPNOW:
      success = initESPNOW();
      if (success) {
        Serial.println("Switched to ESP-NOW mode");
      }
      break;
    case MODE_OFF:
      Serial.println("Switched to OFF mode");
      break;
  }
  
  if (success) {
    current_mode = new_mode;
    first_init = false;
  } else {
    Serial.println("Failed to switch modes, staying in OFF");
    current_mode = MODE_OFF;
  }
  
  return success;
}

void handleSerialInput() {
  if (!Serial.available()) return;
  
  char firstChar = Serial.peek();
  
  // Mode switching commands
  if (firstChar >= '0' && firstChar <= '2') {
    Mode new_mode = (Mode)(Serial.read() - '0');
    switchToMode(new_mode);
    
    // Clear any remaining characters
    while (Serial.available()) Serial.read();
    return;
  }
  
  // Data commands based on current mode
  switch (current_mode) {
    case MODE_BLE:
      if (firstChar == 'A') {
        handleBLEData();
      } else {
        // Clear invalid input
        while (Serial.available()) Serial.read();
      }
      break;
      
    case MODE_ESPNOW:
      if (firstChar == 'B' || firstChar == 'C') {
        handleESPNOWData();
      } else {
        // Clear invalid input
        while (Serial.available()) Serial.read();
      }
      break;
      
    default:
      // Clear any input in OFF mode
      while (Serial.available()) Serial.read();
      break;
  }
}

void handleBLEData() {
  if (Serial.available() && Serial.peek() == 'A') {
    String data = Serial.readString();
    data = data.substring(1); // Remove 'A' prefix
    data.trim();
    
    if (ble_connected && ble_sensor) {
      ble_sensor->setValue(data.c_str());
      ble_sensor->notify();
      Serial.println("BLE data sent: " + data);
    } else {
      Serial.println("BLE not connected or not ready");
    }
  }
}

void handleESPNOWData() {
  if (Serial.available()) {
    char command = Serial.read();
    myData.alert = (command == 'C') ? 1 : 0;
    
    // Clear remaining input
    while (Serial.available()) Serial.read();
    
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));
    
    if (result == ESP_OK) {
      Serial.println("ESP-NOW data sent successfully");
    } else {
      Serial.println("ESP-NOW send failed");
    }
  }
}

void handleBLEConnection() {
  // Handle BLE disconnection
  if (!ble_connected && old_ble_connected) {
    Serial.println("BLE client disconnected - restarting advertising");
    delay(500);
    if (ble_server) {
      ble_server->startAdvertising();
    }
    old_ble_connected = ble_connected;
  }
  
  // Handle BLE connection
  if (ble_connected && !old_ble_connected) {
    Serial.println("BLE client connected");
    old_ble_connected = ble_connected;
  }
}

void loop() {
  // Handle serial input
  handleSerialInput();
  
  // Handle BLE connection state changes
  if (current_mode == MODE_BLE) {
    handleBLEConnection();
  }
  
  // Status output (reduce frequency to avoid spam)
  static unsigned long lastStatus = 0;
  if (millis() - lastStatus > 5000) {
    Serial.print("Current mode: ");
    switch (current_mode) {
      case MODE_OFF: Serial.println("OFF"); break;
      case MODE_BLE: 
        Serial.print("BLE"); 
        Serial.println(ble_connected ? " (Connected)" : " (Advertising)");
        break;
      case MODE_ESPNOW: Serial.println("ESP-NOW"); break;
    }
    lastStatus = millis();
  }
  
  delay(100);
}
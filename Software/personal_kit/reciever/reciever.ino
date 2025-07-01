#include <WiFi.h>
#include <esp_now.h>
//E4:65:B8:B8:93:04

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  float longitude;
  float latitude;
  float measurements[12];
  bool alert;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
  
  //A<lon>,<lat>+<temp>+<humid>+<press>A
  Serial.print("A");
  Serial.print(myData.longitude, 4);
  Serial.print(",");
  Serial.print(myData.latitude, 4);
  for (int i = 0; i < 12; i++) {
    Serial.print("+");
    Serial.print(myData.measurements[i], 2);
  }
  Serial.println("A");
}
 
void setup() {
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}
 
void loop() {
  
}
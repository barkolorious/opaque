#include <ESP8266WiFi.h>
#include <espnow.h>
//5C:CF:7F:16:75:9A

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  float longitude;
  float latitude;
  float measurements[12];
  bool alert;
} struct_message;

int LED_pin = 1;
int trigger_pin = 3;

// Create a struct_message called myData
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
}
 
void setup() {
  pinMode(trigger_pin, OUTPUT);
  pinMode(LED_pin, OUTPUT);
  digitalWrite(LED_pin, 1);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
  Serial.begin(115200);
}
 
void loop() {
  digitalWrite(LED_pin, !myData.alert);
  digitalWrite(trigger_pin, myData.alert);
}
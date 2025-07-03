/////////////////////////////////// ESP-NOW ////////////////////////////////////
void espnow_init (void) {
  Serial.print(F("ESP-NOW starting... "));
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    nextion_display_error(F("failed"));
    for (;;) {}
  }

  esp_now_register_send_cb(espnow_on_data_sent);

  espnow_info.channel = 0;  
  espnow_info.encrypt = false;
  memcpy(espnow_info.peer_addr, espnow_prmd_addr, 6);
  
  if (esp_now_add_peer(&espnow_info) != ESP_OK){
    nextion_display_error(F("failed"));
    for (;;) {}
  }

  memcpy(espnow_info.peer_addr, espnow_reciever_addr, 6);
  
  if (esp_now_add_peer(&espnow_info) != ESP_OK){
    nextion_display_error(F("failed"));
    for (;;) {}
  }

  Serial.println(F("done!"));
}

void espnow_update (void) {
  espnow_data.alert = is_alarm || (millis() - last_alarm_time < 5000);
  espnow_data.longitude = neo6m_lng;
  espnow_data.latitude  = neo6m_lat;
  for (int i = 0; i < 12; i++) espnow_data.measurements[i] = measurements[i];

  esp_err_t result = esp_now_send(0, (uint8_t*) &espnow_data, sizeof(espnow_data));
  if (result != ESP_OK) Serial.println("ESP-NOW: Error sending the data");
}

void espnow_on_data_sent (const uint8_t* mac_addr, esp_now_send_status_t status) {
  Serial.print("ESP-NOW packet send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
/////////////////////////////////// ESP-NOW ////////////////////////////////////
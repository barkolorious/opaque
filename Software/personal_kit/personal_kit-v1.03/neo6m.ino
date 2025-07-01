//////////////////////////////////// NEO-6M ////////////////////////////////////
void neo6m_init (void) {
  Serial.print(F("NEO6M starting... "));
  neo6m_serial.begin(9600, SERIAL_8N1, UART2_RX, UART2_TX);
  if (!neo6m_serial) {
    nextion_display_error(F("failed"));
    for (;;) {}
  }
  Serial.println(F("done!"));
}

void neo6m_read (void) {
  unsigned long start = millis();

  while (millis() - start < 1000) {
    while (neo6m_serial.available() > 0) {
      neo6m.encode(neo6m_serial.read());
    }
    if (neo6m.location.isUpdated()) {
      neo6m_lat = neo6m.location.lat();
      neo6m_lng = neo6m.location.lng();
      neo6m_alt = neo6m.altitude.meters();
    }
  }
}
//////////////////////////////////// NEO-6M ////////////////////////////////////
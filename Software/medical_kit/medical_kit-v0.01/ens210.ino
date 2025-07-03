//////////////////////////////////// ENS210 ////////////////////////////////////
void ens210_init (void) {
  Serial.print(F("ENS210 starting... "));
  ens210.begin();
  ens210.init();
  if (ens210.isConnected() == false) {
    nextion_display_error(F("failed"));
    for (;;) {}
  }
  Serial.println(F("done!"));

  Serial.print(F("ENS210 initializing..."));
  while (ens210.startContinuousMeasure() != RESULT_OK) {
    Serial.print(F("."));
    delay(ENS21X_SYSTEM_TIMING_BOOTING);
  }
  Serial.println(F(" done!"));
}

void ens210_read (void) {
  if (ens210.update() == RESULT_OK) {
    ens210_temperature = ens210.getTempCelsius();
    ens210_humidity    = ens210.getHumidityPercent();
  }
}
//////////////////////////////////// ENS210 ////////////////////////////////////
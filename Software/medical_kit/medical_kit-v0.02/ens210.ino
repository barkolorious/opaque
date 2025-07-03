//////////////////////////////////// ENS210 ////////////////////////////////////
void ens210_init (void) {
  ssd1306_new_screen();
  ssd1306_print(F("ENS210 starting... "));
  ens210.begin();
  ens210.init();
  if (ens210.isConnected() == false) {
    ssd1306_println(F("failed"));
    for (;;) {}
  }
  ssd1306_println(F("done!"));

  ssd1306_print(F("ENS210 initializing..."));
  while (ens210.startContinuousMeasure() != RESULT_OK) {
    ssd1306_print(F("."));
    delay(ENS21X_SYSTEM_TIMING_BOOTING);
  }
  ssd1306_println(F(" done!"));
}

void ens210_read (void) {
  if (ens210.update() == RESULT_OK) {
    ens210_temperature = ens210.getTempCelsius();
    ens210_humidity    = ens210.getHumidityPercent();
  }
}
//////////////////////////////////// ENS210 ////////////////////////////////////
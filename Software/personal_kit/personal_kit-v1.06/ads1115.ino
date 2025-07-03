/////////////////////////////////// ADS1115 ////////////////////////////////////
void ads1115_init (void) {
  Serial.print(F("ADS1115 starting... "));
  if (!ads1115.begin()) {
    nextion_display_error(F("failed"));
    for (;;) {}
  }
  Serial.println(F("done!"));
}

float ads1115_read (uint8_t pin) {
  int16_t adc = ads1115.readADC_SingleEnded(0);
  return ads1115.computeVolts(adc);
}
/////////////////////////////////// ADS1115 ////////////////////////////////////
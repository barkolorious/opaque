/////////////////////////////////// SD Card ////////////////////////////////////
void sd_card_init (void) {
  Serial.print(F("SD Card starting... "));
  SPI.begin(VSPI_CLK, VSPI_MISO, VSPI_MOSI, VSPI_CS);
  if (!SD.begin(5)) {
    nextion_display_error(F("failed"));
    for (;;) {}
  }
  Serial.println(F("done!"));

  Serial.print(F("SD Card initializing... "));
  sd_card_type = SD.cardType();
  if (sd_card_type == CARD_NONE) {
    nextion_display_error(F("failed"));
    for (;;) {}
  }
  Serial.println(F("done!"));
}

void sd_card_write_file (fs::FS &fs, const char *path, const char *message) {
  File file = fs.open(path, FILE_APPEND);

  if (!file) {
    nextion_display_error(F("ERR: SD Card writing failed"));
    return;
  }

  file.println(message);
  file.close();
}
/////////////////////////////////// SD Card ////////////////////////////////////
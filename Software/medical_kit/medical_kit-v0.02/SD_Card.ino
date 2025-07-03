/////////////////////////////////// SD Card ////////////////////////////////////
void sd_card_init (void) {
  ssd1306_new_screen();
  ssd1306_print(F("SD Card starting... "));
  SPI.begin(VSPI_CLK, VSPI_MISO, VSPI_MOSI, VSPI_CS);
  if (!SD.begin(5)) {
    ssd1306_println(F("failed"));
    for (;;) {}
  }
  ssd1306_println(F("done!"));

  ssd1306_print(F("SD Card initializing... "));
  sd_card_type = SD.cardType();
  if (sd_card_type == CARD_NONE) {
    ssd1306_println(F("failed"));
    for (;;) {}
  }
  ssd1306_println(F("done!"));
}

void sd_card_write_file (fs::FS &fs, const char *path, const char *message) {
  File file = fs.open(path, FILE_APPEND);

  if (!file) {
    ssd1306_println(F("ERR: SD Card writing failed"));
    return;
  }

  file.println(message);
  file.close();
}
/////////////////////////////////// SD Card ////////////////////////////////////
/////////////////////////////////// PMS7003 ////////////////////////////////////
void pms7003_init (void) {
  ssd1306_new_screen();
  ssd1306_print(F("PMS7003 starting... "));
  pms7003.init();
  ssd1306_println(F("done!"));
}

void pms7003_read (void) {
  if (pms7003_last_read == -1) pms7003_last_read = millis();
  if (millis() - pms7003_last_read < 5000) return;
  pms7003_last_read = millis();

  pms7003.read();
  if (pms7003) {
    pms7003_PM1   = pms7003.pm01;
    pms7003_PM2p5 = pms7003.pm25;
    pms7003_PM10  = pms7003.pm10;
    pms7003_N0p3  = pms7003.n0p3;
    pms7003_N0p5  = pms7003.n0p5;
    pms7003_N1    = pms7003.n1p0;
    pms7003_N2p5  = pms7003.n2p5;
    pms7003_N5    = pms7003.n5p0;
    pms7003_N10   = pms7003.n10p0;
  } else {
    ssd1306_print(F("ERR: PMS7003 "));
    switch (pms7003.status) {
      case pms7003.OK: break;
      case pms7003.ERROR_TIMEOUT:     ssd1306_println(F(PMS_ERROR_TIMEOUT));     break;
      case pms7003.ERROR_MSG_UNKNOWN: ssd1306_println(F(PMS_ERROR_MSG_UNKNOWN)); break;
      case pms7003.ERROR_MSG_HEADER:  ssd1306_println(F(PMS_ERROR_MSG_HEADER));  break;
      case pms7003.ERROR_MSG_BODY:    ssd1306_println(F(PMS_ERROR_MSG_BODY));    break;
      case pms7003.ERROR_MSG_START:   ssd1306_println(F(PMS_ERROR_MSG_START));   break;
      case pms7003.ERROR_MSG_LENGTH:  ssd1306_println(F(PMS_ERROR_MSG_LENGTH));  break;
      case pms7003.ERROR_MSG_CKSUM:   ssd1306_println(F(PMS_ERROR_MSG_CKSUM));   break;
      case pms7003.ERROR_PMS_TYPE:    ssd1306_println(F(PMS_ERROR_PMS_TYPE));    break;
    }
  }
}
/////////////////////////////////// PMS7003 ////////////////////////////////////
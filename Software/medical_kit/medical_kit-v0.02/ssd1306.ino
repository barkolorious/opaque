/////////////////////////////////// SSD1306 ////////////////////////////////////
void ssd1306_init (void) {
  if(!ssd1306.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;) {}
  }

  ssd1306.setFont(&Org_01);
  ssd1306.setTextColor(SSD1306_WHITE);
  ssd1306.setTextSize(1);
  ssd1306.clearDisplay();
  ssd1306.display();
}

void ssd1306_opening_animation (void) {
  ssd1306.clearDisplay();
  ssd1306.display();
  for (int i = 0; i < 6; i++) {
    delay(100);
    ssd1306.drawBitmap(2 + 21 * i, 27, logo_o, SSD1306_LOGO_O_WIDTH, SSD1306_LOGO_O_HEIGHT, SSD1306_WHITE);
    ssd1306.display();
  }
  delay(150);
  ssd1306.clearDisplay();
  ssd1306.drawBitmap(2, 27, logo_opaque_frame0, 125, 15, SSD1306_WHITE);
  ssd1306.display();
  delay(75);
  ssd1306.clearDisplay();
  ssd1306.drawBitmap(2, 27, logo_opaque, 125, 15, SSD1306_WHITE);
  ssd1306.display();
  delay(500);
}

void ssd1306_display_datetime (void) {
  ds3231_read();
  ssd1306.clearDisplay();
  
  int16_t x, y;
  uint16_t w, h;
  ssd1306.setTextSize(4);
  ssd1306.getTextBounds(ds3231_curr_time.c_str(), 0, 0, &x, &y, &w, &h);
  ssd1306.setCursor((ssd1306.width() - w) / 2, (ssd1306.height() - h) / 2);
  ssd1306.println(ds3231_curr_time);
  ssd1306.setTextSize(1);
  ssd1306.getTextBounds(ds3231_curr_date.c_str(), 0, (ssd1306.height() + h) / 2, &x, &y, &w, &h);
  ssd1306.setCursor((ssd1306.width() - w) / 2, y);
  ssd1306.println(ds3231_curr_date);

  ssd1306.display();
}

void ssd1306_print (String text) {
  Serial.print(text);
  ssd1306.print(text);
  ssd1306.display();
}

void ssd1306_println (String text) {
  Serial.println(text);
  ssd1306.println(text);
  ssd1306.display();
}

void ssd1306_new_screen (void) {
  ssd1306.clearDisplay();
  ssd1306.setCursor(0, 7);
  ssd1306.display();
}

void ssd1306_update (void) {
  ssd1306_new_screen();
  ssd1306.setTextWrap(false);
  ssd1306.drawBitmap(8, 16, tags, 81, 40, 1);
  
  ssd1306.setCursor(8, 7);
  ssd1306.print(ds3231_curr_time.c_str());
  ssd1306.setCursor(101, 7);
  ssd1306.print("PIL");

  ssd1306.setCursor(34, 20);
  ssd1306.print(ens160_eCO2); // CO2
  ssd1306.setCursor(34, 27);
  ssd1306.print(ens160_tVOC); // VOC
  ssd1306.setCursor(34, 34);
  ssd1306.print(ens160_AQI); // AQI
  ssd1306.setCursor(34, 41);
  ssd1306.print(ens210_temperature); // Temp
  ssd1306.setCursor(34, 48);
  ssd1306.print(ens210_humidity); // Humid
  ssd1306.setCursor(34, 55);
  ssd1306.print(mq131_O3); // O3
  
  ssd1306.setCursor(94, 20);
  ssd1306.print(pms7003_PM1); // PM1
  ssd1306.setCursor(94, 27);
  ssd1306.print(pms7003_PM2p5); // PM2.5
  ssd1306.setCursor(94, 34);
  ssd1306.print(pms7003_PM10); // PM10
  ssd1306.setCursor(94, 41);
  ssd1306.print(mics6814_NH3); // NH3
  ssd1306.setCursor(94, 48);
  ssd1306.print(mics6814_NO2); // NOx
  ssd1306.setCursor(94, 55);
  ssd1306.print(mics6814_CO); // CO

  ssd1306.display();
}
/////////////////////////////////// SSD1306 ////////////////////////////////////
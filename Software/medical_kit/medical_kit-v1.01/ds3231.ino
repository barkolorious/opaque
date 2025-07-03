//////////////////////////////////// DS3231 ////////////////////////////////////
void ds3231_init (void) {
  ssd1306_new_screen();
  ssd1306_print(F("DS3231 starting... "));
  if (!ds3231.begin()) {
    ssd1306_println(F("failed"));
    for (;;) {}
  }
  ssd1306_println(F("done!"));

  ssd1306_print(F("DS3231 initializing... "));
  ds3231.adjust(DateTime(F(__DATE__), F(__TIME__)));
  ssd1306_println(F("done!"));
}

void ds3231_read (void) {
  DateTime now = ds3231.now();
  ds3231_get_date(now);
  ds3231_get_time(now);
}

void ds3231_get_date (DateTime now) {
  ds3231_curr_date = String(monthsOfTheYear[now.month()]);
  ds3231_curr_date += String(' ');
  ds3231_curr_date += String(now.day(), DEC);
  ds3231_curr_date += String(',');
  ds3231_curr_date += daysOfTheWeek[now.dayOfTheWeek()];
}

void ds3231_get_time (DateTime now) {
  ds3231_curr_time = String(now.hour(), DEC);
  ds3231_curr_time += String(':');
  ds3231_curr_time += String(now.minute(), DEC);
}
//////////////////////////////////// DS3231 ////////////////////////////////////
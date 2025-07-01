/////////////////////////////////// NEXTION ////////////////////////////////////
void nextion_init (void) {
  Serial.print(F("Nextion starting... "));
  pinMode(UART3_RX, INPUT);
  pinMode(UART3_TX, OUTPUT);
  nextion.begin(9600);
  nextion.setTimeout(100);
  if (!nextion) {
    nextion_display_error(F("failed"));
  }
  Serial.println(F("done!"));
}

void nextion_display_error (String text) {
  Serial.println(text);
}

void nextion_update (void) {
  nextion_update_measurements();
  nextion_update_battery();
  nextion_update_clock();
  nextion_update_bluetooth();
}

void nextion_update_measurements (void) {
  for (int i = 0; i < 12; i++) {
    nextion.print(nextion_measurement_objs[i] + measurements[i] + "\"" + NEXTION_COMMAND_END);
  }
}

void nextion_update_battery (void) {
  uint8_t battery_level = 150;
  uint8_t battery_image = 0;
  if      (battery_level <= 10)  battery_image = 1;
  else if (battery_level <= 40)  battery_image = 2;
  else if (battery_level <= 60)  battery_image = 3;
  else if (battery_level <= 80)  battery_image = 4;
  else if (battery_level <= 100) battery_image = 5;

  for (int i = 0; i < 6; i++) {
    if (battery_image) nextion.print(nextion_pages[i] + ".battery.txt=\"%" + battery_level + "\"" + NEXTION_COMMAND_END);
    else               nextion.print(nextion_pages[i] + ".battery.txt=\"SARJ\"" + NEXTION_COMMAND_END);
    nextion.print(nextion_pages[i] + ".battery_lvl.pic=" + battery_image + NEXTION_COMMAND_END);
  }
}

void nextion_update_clock (void) {
  for (int i = 0; i < 6; i++) 
    nextion.print(nextion_pages[i] + ".clock.txt=\"" + ds3231_curr_time + "\"" + NEXTION_COMMAND_END);
  nextion.print("home_page.date.txt=\"" + ds3231_curr_date + "\"" + NEXTION_COMMAND_END);
}

void nextion_update_bluetooth (void) {
  uint8_t bluetooth_connected = 1;
  for (int i = 0; i < 6; i++)
    nextion.print(nextion_pages[i] + ".bluetooth.pic=" + ((bluetooth_connected) ? 6 : 7) + NEXTION_COMMAND_END);
}

void nextion_read (void) {
  if (nextion.available()) {
    String nextion_input = "";
    nextion_input = nextion.readString();
    Serial.println(nextion_input);
    char mode = nextion_input[0];

    if (mode != 'A' && mode != 'B') {
      Serial.println("Nextion error");
      return;
    }
    
    int j = 1;
    switch (mode) {
      case 'A':
        for (int i = 0; i < 12; i++) {
          String temp = "";
          while (j < nextion_input.length() && nextion_input[j] != '+' && nextion_input[j] != 'A') temp += nextion_input[j++];
          j++;
          alarm_values[i] = temp.toFloat();
        }
        nextion.print("alarm_1.save_button.txt=\"Kaydedildi\"\xFF\xFF\xFF");
        nextion.print("alarm_2.save_button.txt=\"Kaydedildi\"\xFF\xFF\xFF");
        nextion.print("alarm_1.button_reset.en=1\xFF\xFF\xFF");
        nextion.print("alarm_2.button_reset.en=1\xFF\xFF\xFF");
        break;
      case 'B':
        int temp_arr[5];
        for (int i = 0; i < 5; i++) {
          String temp = "";
          while (j < nextion_input.length() && nextion_input[j] != '+' && nextion_input[j] != 'B') temp += nextion_input[j++];
          j++;
          temp_arr[i] = temp.toInt();
        }
        ds3231_curr_date = temp_arr[1];
        ds3231_curr_date += monthsOfTheYear[temp_arr[0]];
        ds3231_curr_date += ", ";
        ds3231_curr_date += daysOfTheWeek[temp_arr[4]];
        ds3231_curr_time = temp_arr[2];
        ds3231_curr_time += ":";
        ds3231_curr_time += temp_arr[3];
        break;
    }
  }  
}
/////////////////////////////////// NEXTION ////////////////////////////////////
#include <SoftwareSerial.h>
#define UART3_RX        25 /* GPIO25 (SoftwareSerial [UART3] RX) -- Nextion TX */
#define UART3_TX        26 /* GPIO26 (SoftwareSerial [UART3] TX) -- Nextion RX */
#define NEXTION_INT_PIN 35 /* GPIO35 (Int) -- Nextion */

SoftwareSerial nextion(UART3_RX, UART3_TX);

String meas_objs[12] = {"measurement_1.temp_value.txt=\"", 
                        "measurement_1.humid_value.txt=\"", 
                        "measurement_1.co2_value.txt=\"", 
                        "measurement_1.tvoc_value.txt=\"", 
                        "measurement_1.aqi_value.txt=\"", 
                        "measurement_1.o3_value.txt=\"", 
                        
                        "measurement_2.nh3_value.txt=\"", 
                        "measurement_2.no2_value.txt=\"", 
                        "measurement_2.co_value.txt=\"", 
                        "measurement_2.pm0p1_value.txt=\"", 
                        "measurement_2.pm2p5_value.txt=\"", 
                        "measurement_2.pm10_value.txt=\""};



String pages[6] = {"home_page", "measurement_1", "measurement_2", "alarm_1", "alarm_2", "settings"};

float alarms[12] = {0,0,0,0,0,0,0,0,0,0,0,0};

void setup() {
  // put your setup code here, to run once:
  pinMode(UART3_RX, INPUT);
  pinMode(UART3_TX, OUTPUT);
  nextion.begin(9600);
  Serial.begin(9600);
  nextion.setTimeout(100);
  Serial.println("started");
}

void loop() {
  // put your main code here, to run repeatedly:
  String instring = "", part = "";
  int battery_level, battery_img;
  int j = 0;
  while (Serial.available()) {
    instring = Serial.readString();
    Serial.println(instring);
    char mode = instring[0];
    switch (mode) {
      case 'M':
        Serial.println(mode);
        j = 1;
        for (int i = 0; i < 12; i++) {
          part = "";
          while (j < instring.length() && instring[j] != '+' && instring[j] != 'M') part += instring[j++];
          j++;
          nextion.print(meas_objs[i] + part + "\"\xFF\xFF\xFF");
          Serial.println(meas_objs[i] + part + "\"\xFF\xFF\xFF");
        }
        Serial.println(mode);
        break;
      case 'P':
        //P150P PİL
        Serial.println(mode);
        battery_level = instring.substring(1, instring.length() - 2).toInt();
        if (battery_level < 10) battery_img = 1;
        else if (battery_level < 40) battery_img = 2;
        else if (battery_level < 60) battery_img = 3;
        else if (battery_level < 80) battery_img = 4;
        else if (battery_level <= 100) battery_img = 5;
        else battery_img = 0;

        for (int i = 0; i < 6; i++) {
          if (battery_img) {
            nextion.print(pages[i] + ".battery.txt=\"%" + battery_level + "\"\xFF\xFF\xFF");
            Serial.println(pages[i] + ".battery.txt=\"%" + battery_level + "\"\xFF\xFF\xFF");
          } else {
            nextion.print(pages[i] + ".battery.txt=\"SARJ\"\xFF\xFF\xFF");
            Serial.println(pages[i] + ".battery.txt=\"SARJ\"\xFF\xFF\xFF");
          }
          nextion.print(pages[i] + ".battery_lvl.pic=" + battery_img + "\xFF\xFF\xFF");
          Serial.println(pages[i] + ".battery_lvl.pic=" + battery_img + "\xFF\xFF\xFF");
        }
        Serial.println(mode);
        break;
      case 'S':
        // S20:04S saat
        Serial.println(mode);
        for (int i = 0; i < 6; i++) {
          nextion.print(pages[i] + ".clock.txt=\"" + instring.substring(1, instring.length() - 2) + "\"\xFF\xFF\xFF");
          Serial.println(pages[i] + ".clock.txt=\"" + instring.substring(1, instring.length() - 2) + "\"\xFF\xFF\xFF");
        }
        Serial.println(mode);
        break;
      case 'B':
        Serial.println(mode);
        for (int i = 0; i < 6; i++) {
          nextion.print(pages[i] + ".bluetooth.pic=" + ((instring[1] == '1') ? 6 : 7) + "\xFF\xFF\xFF");
          Serial.println(pages[i] + ".bluetooth.pic=" + ((instring[1] == '1') ? 6 : 7) + "\xFF\xFF\xFF");
        }
        Serial.println(mode);
        break;
    }
  }

  if (nextion.available()) {
    instring = nextion.readString();
    Serial.println(instring);
    if (instring[0] == 'A') {
      Serial.println("1");
      nextion.print("alarm_1.save_button.txt=\"Kaydedildi\"\xFF\xFF\xFF");
      nextion.print("alarm_2.save_button.txt=\"Kaydedildi\"\xFF\xFF\xFF");
      nextion.print("alarm_1.button_reset.en=1\xFF\xFF\xFF");
      nextion.print("alarm_2.button_reset.en=1\xFF\xFF\xFF");
      Serial.println("5");
    } else {
      Serial.println("error");
    }
  }
}
/*
+---------+--------+
| SD Card | ESP32  |
+=========+========+
| GND     | GND    |
| VCC     | 5V     |
| MISO    | GPIO19 |
| MOSI    | GPIO23 |
| CLK     | GPIO18 |
| CS      | GPIO5  |
+---------+--------+

https://randomnerdtutorials.com/esp32-microsd-card-arduino/
*/

#include "FS.h"
#include "SD.h"
#include "SPI.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.println("started");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial0.available()) {
    String data = Serial0.readStringUntil('\n');
    Serial0.println(data);
    writeFile(SD, "/test.txt", data.c_str());
  }

}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  if (file.println(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

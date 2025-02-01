/*
+--------+-------+
| NEO-6M | ESP32 |
+========+=======+
| VCC    | 3V3   |
| RX     | GPIO2 |
| TX     | GPIO4 |
| GND    | GND   |
+--------+--------+

https://randomnerdtutorials.com/esp32-neo-6m-gps-module-arduino/
https://hackaday.com/2017/08/17/secret-serial-port-for-arduinoesp32/
*/

#include <TinyGPS++.h>

#define RXD1 4
#define TXD1 2

#define GPS_BAUD 9600

TinyGPSPlus gps;

HardwareSerial gpsSerial(1);

void setup() {
  Serial.begin(115200);
  
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD1, TXD1);
  Serial.println("Serial 1 started at 9600 baud rate");
}

void loop() {
  unsigned long start = millis();

  while (millis() - start < 1000) {
    while (gpsSerial.available() > 0) {
      gps.encode(gpsSerial.read());
    }
    if (gps.location.isUpdated()) {
      Serial.print("LAT: ");
      Serial.println(gps.location.lat(), 6);
      Serial.print("LONG: "); 
      Serial.println(gps.location.lng(), 6);
      Serial.print("SPEED (km/h) = "); 
      Serial.println(gps.speed.kmph()); 
      Serial.print("ALT (min)= "); 
      Serial.println(gps.altitude.meters());
      Serial.print("HDOP = "); 
      Serial.println(gps.hdop.value() / 100.0); 
      Serial.print("Satellites = "); 
      Serial.println(gps.satellites.value()); 
      Serial.print("Time in UTC: ");
      Serial.println(String(gps.date.year()) + "/" + String(gps.date.month()) + "/" + String(gps.date.day()) + "," + String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second()));
      Serial.println("");
    }
  }
}
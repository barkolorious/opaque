// now.pde
// Prints a snapshot of the current date and time along with the UNIX time
// Modified by Andy Wickert from the JeeLabs / Ladyada RTC library examples
// 5/15/11

#include <Wire.h>
#include <DS3231.h>

RTClib rtc;

void setup () {
    Serial.begin(57600);
    Wire.begin();
    delay(500);
    Serial.println("Nano Ready!");
}

void loop () {
  delay(1000);
    
  DateTime now = rtc.now();

  String date, time;

  time = String(now.hour(), DEC);
  time += String(':');
  time += String(now.minute(), DEC);

  date = String(now.day(), DEC);
  date += String('/');
  date += String(now.month(), DEC);
  date += String('/');
  date += String(now.year(), DEC);

  Serial.print(date);
  Serial.print(' ');
  Serial.println(time);
}
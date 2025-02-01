/*
+-----------------+--------+
| ENS160 + ENS210 | ESP32  |
+=================+========+
| GND             | GND    |
| VCC             | 3V3    |
| SDA             | GPIO21 |
| SCL             | GPIO22 |
+-----------------+--------+
*/

#include <Arduino.h>
#include <Wire.h>
#include "ens210.h"
#include "SparkFun_ENS160.h"

using namespace ScioSense;

ENS210 ens210;
SparkFun_ENS160 ens160; 
int ensStatus; 

void setup() {
	Serial.begin(115200);
  Wire.begin();
  ens210.begin();

  if (ens210.isConnected() == false) {
    Serial.println("Error -- The ENS210 is not connected.");
    return;
  }

  ens210.reset();

  Serial.print("Starting continous mode..");
  while (ens210.startContinuousMeasure() != ENS210::Result::STATUS_OK) {
    Serial.print(".");
    delay(ENS210::SystemTiming::BOOTING);
  }
  Serial.println(" Done!");

  Serial.println();
  Serial.println("----------------------------------------");

  if (!ens160.begin() ) {
		Serial.println("Could not communicate with the ENS160, check wiring.");
		return;
	}

  Serial.println("Example 1 Basic Example.");

	// Reset the indoor air quality sensor's settings.
	if( ens160.setOperatingMode(SFE_ENS160_RESET) ) Serial.println("Ready.");

	delay(100);

  ens160.setOperatingMode(SFE_ENS160_STANDARD);
  ensStatus = ens160.getFlags();
	Serial.print("Gas Sensor Status Flag (0 - Standard, 1 - Warm up, 2 - Initial Start Up): ");
	Serial.println(ensStatus);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (ens210.update() == ENS210::Result::STATUS_OK) {
    float temperatureCelsius = ens210.getTempCelsius();
    float humidityPercent    = ens210.getHumidityPercent();

    Serial.print("T:");
    Serial.print(temperatureCelsius);
    Serial.print("°C\t");

    Serial.print("Humidity:");
    Serial.print(humidityPercent);
    Serial.println("%");
    ens210.getStatusH();
  }

  if( ens160.checkDataStatus() )	{
		Serial.print("AQI (1-5) : ");
		Serial.println(ens160.getAQI());

		Serial.print("tVOC: ");
		Serial.print(ens160.getTVOC());
		Serial.println("ppb");

		Serial.print("eCO2: ");
		Serial.print(ens160.getECO2());
		Serial.println("ppm");

	  Serial.print("Gas Sensor Status Flag (0 - Standard, 1 - Warm up, 2 - Initial Start Up): ");
    Serial.println(ens160.getFlags());

		Serial.println();
	}

	delay(200);
}

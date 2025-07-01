#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads;

#define MQ131_DEFAULT_HI_CONCENTRATION_R0           235.00            // Default R0 for high concentration MQ131
#define MQ131_DEFAULT_HI_CONCENTRATION_TIME2READ    80                // Default time to read before stable signal for high concentration MQ131

uint32_t MQ131_R_load = 22000;
int8_t temperatureCelsuis = 26;
uint8_t humidityPercent = 20;
float O3 = 0;

float MQ131_getEnvCorrectRatio() {
 	// Select the right equation based on humidity
 	// If default value, ignore correction ratio
 	if(humidityPercent == 60 && temperatureCelsuis == 20) {
 		return 1.0;
 	}
 	// For humidity > 75%, use the 85% curve
 	if(humidityPercent > 75) {
    // R^2 = 0.996
   	return -0.0103 * temperatureCelsuis + 1.1507;
 	}
 	// For humidity > 50%, use the 60% curve
 	if(humidityPercent > 50) {
 		// R^2 = 0.9976
 		return -0.0119 * temperatureCelsuis + 1.3261;
 	}

 	// Humidity < 50%, use the 30% curve
  // R^2 = 0.9986
 	return -0.0141 * temperatureCelsuis + 1.5623;
}

void mq131_read () {
  int16_t ads1115_adc_3 = ads.readADC_SingleEnded(3);
  float mq131_voltage = ads.computeVolts(ads1115_adc_3);
  //float R_s = (5.0 / mq131_voltage - 1.0) * MQ131_R_load;
  float R_s = (5.0 / mq131_voltage - 1.0) * MQ131_R_load;
  float ratio = R_s / 15000 * MQ131_getEnvCorrectRatio();

  O3 = (8.37768358 * pow(ratio, 2.30375446) - 8.37768358);
  Serial.print("Volt:"); Serial.print(mq131_voltage);
  Serial.print(",O3:"); Serial.print(O3);
  Serial.print(",Ratio:"); Serial.print(ratio);
  Serial.print(",ads:"); Serial.print(ads1115_adc_3);
  Serial.print(",R_s:"); Serial.println(R_s);
}

void setup() {
  Serial.begin(115200);
 
  Serial.println("Getting single-ended readings from AIN0..3");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");

  if (!ads.begin())
  {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
}

void loop() {
  mq131_read();
}

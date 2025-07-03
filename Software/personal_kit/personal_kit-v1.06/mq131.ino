//////////////////////////////////// MQ131 /////////////////////////////////////
void mq131_init (void) {
  Serial.println(F("MQ131 starting... done!"));
}

void mq131_read (void) {
  float voltage = ads1115_read(MQ131_ADS1115_PIN);
  float R_s = (5.0 / voltage - 1.0) * MQ131_R_LOAD;
  float ratio = R_s / 15000 * mq131_get_environment_correction_ratio();
	Serial.print("R_s: ");
	Serial.print(R_s);
	Serial.print(", voltage: ");
	Serial.print(voltage);
	Serial.print(", env: ");
	Serial.print(mq131_get_environment_correction_ratio());
	Serial.print(", ratio: ");
	Serial.print(ratio);

	if (std::isnan(8.1399  * pow(ratio, 2.3297))) mq131_O3 = 0;
	else 																						mq131_O3 = (8.1399  * pow(ratio, 2.3297));
	
	Serial.print(", measurements 5: ");
	Serial.print(measurements[5]);
	Serial.print(", o3: ");
	Serial.println(mq131_O3);
}

float mq131_get_environment_correction_ratio (void) {
  // Select the right equation based on humidity
 	// If default value, ignore correction ratio
 	if(ens210_humidity == 60 && ens210_temperature == 20) {
 		return 1.0;
 	}
 	// For humidity > 75%, use the 85% curve
 	if(ens210_humidity > 75) {
    // R^2 = 0.996
   	return -0.0103 * ens210_temperature + 1.1507;
 	}
 	// For humidity > 50%, use the 60% curve
 	if(ens210_humidity > 50) {
 		// R^2 = 0.9976
 		return -0.0119 * ens210_temperature + 1.3261;
 	}

 	// Humidity < 50%, use the 30% curve
  // R^2 = 0.9986
 	return -0.0141 * ens210_temperature + 1.5623;
}
//////////////////////////////////// MQ131 /////////////////////////////////////
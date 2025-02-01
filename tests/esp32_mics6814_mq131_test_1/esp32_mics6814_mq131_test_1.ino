#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads;

/*-------------------------MQ131-----------------------------*/

#define MQ131_DEFAULT_HI_CONCENTRATION_R0           235.00            // Default R0 for high concentration MQ131
#define MQ131_DEFAULT_HI_CONCENTRATION_TIME2READ    80                // Default time to read before stable signal for high concentration MQ131

uint32_t MQ131_R_load = 22000;
int8_t temperatureCelsuis = 26;
uint8_t humidityPercent = 20;
float O3 = 0;

/*-----------------------MiCS6814----------------------------*/

enum channel {CH_NH3, CH_RED, CH_OX};
typedef enum channel channel_t;

enum gas {CO, NO2, NH3, C3H8, C4H10, CH4, H2, C2H5OH};
typedef enum gas gas_t;

#define NH3PIN 1
#define COPIN 0
#define OXPIN 2

uint16_t NH3baseR;
uint16_t REDbaseR;
uint16_t OXbaseR;

/**
   Requests the current resistance for a given channel
   from the sensor. The value is an ADC value between
   0 and 1024.

   @param channel
          The channel to read the base resistance from.
   @return The unsigned 16-bit base resistance
           of the selected channel.
*/
uint16_t getResistance(channel_t channel) {
  unsigned long rs = 0;
  int counter = 0;
  switch (channel) {
    case CH_NH3:
      for(int i = 0; i < 100; i++) {
        rs += ads.readADC_SingleEnded(NH3PIN);
        counter++;
        delay(2);
      }
      return rs/counter;
    case CH_RED:
      for(int i = 0; i < 100; i++) {
        rs += ads.readADC_SingleEnded(COPIN);
        counter++;
        delay(2);
      }
      return rs/counter;
    case CH_OX:      
      for(int i = 0; i < 100; i++) {
        rs += ads.readADC_SingleEnded(OXPIN);
        counter++;
        delay(2);
      }
      return rs/counter;
  }

  return 0;
}

void calibrateMICS() {
  // Continuously measure the resistance,
  // storing the last N measurements in a circular buffer.
  // Calculate the floating average of the last seconds.
  // If the current measurement is close to the average stop.

  // Seconds to keep stable for successful calibration
  // (Keeps smaller than 64 to prevent overflows)
  uint8_t seconds = 10;
  // Allowed delta for the average from the current value
  uint8_t delta = 50;

  // Circular buffer for the measurements
  uint16_t bufferNH3[seconds];
  uint16_t bufferRED[seconds];
  uint16_t bufferOX[seconds];
  // Pointers for the next element in the buffer
  uint8_t pntrNH3 = 0;
  uint8_t pntrRED = 0;
  uint8_t pntrOX = 0;
  // Current floating sum in the buffer
  int32_t fltSumNH3 = 0;
  int32_t fltSumRED = 0;
  int32_t fltSumOX = 0;

  // Current measurements;
  int16_t curNH3;
  int16_t curRED;
  int16_t curOX;

  // Flag to see if the channels are stable
  bool NH3stable = false;
  bool REDstable = false;
  bool OXstable = false;

  // Initialize buffer
  for (int i = 0; i < seconds; ++i) {
    bufferNH3[i] = 0;
    bufferRED[i] = 0;
    bufferOX[i] = 0;
  }

  do {
    // Wait a second
    delay(1000);
    Serial.print(".");
    // Read new resistances
    unsigned long rs = 0;
    delay(50);
    for (int i = 0; i < 3; i++) {
      delay(1);
      rs += ads.readADC_SingleEnded(NH3PIN);
    }
    curNH3 = rs/3;
    /*rs = 0;
    delay(50);
    for (int i = 0; i < 3; i++) {
      delay(1);
      rs += ads.readADC_SingleEnded(COPIN);
    }
    curRED = rs/3;*/
    /*rs = 0;
    delay(50);
    for (int i = 0; i < 3; i++) {
      delay(1);
      rs += ads.readADC_SingleEnded(OXPIN);
    }
    curOX = rs/3;*/

    // Update floating sum by subtracting value
    // about to be overwritten and adding the new value.
    fltSumNH3 = fltSumNH3 + curNH3 - bufferNH3[pntrNH3];
    //fltSumRED = fltSumRED + curRED - bufferRED[pntrRED];
    //fltSumOX = fltSumOX + curOX - bufferOX[pntrOX];

    // Store new measurement in buffer
    bufferNH3[pntrNH3] = curNH3;
    //bufferRED[pntrRED] = curRED;
    //bufferOX[pntrOX] = curOX;

    // Determine new state of flags
    NH3stable = abs(fltSumNH3 / seconds - curNH3) < delta;
    //REDstable = abs(fltSumRED / seconds - curRED) < delta;
    //OXstable = abs(fltSumOX / seconds - curOX) < delta;

    // Advance buffer pointer
    pntrNH3 = (pntrNH3 + 1) % seconds ;
    //pntrRED = (pntrRED + 1) % seconds;
    //pntrOX = (pntrOX + 1) % seconds;

    //Mikä kestää?
    if(!NH3stable) {
      Serial.print("cur");
      Serial.print(curNH3);
      Serial.print(",fltSum");
      Serial.print(fltSumNH3);
      Serial.print(",fltSum_avg");
      Serial.println(fltSumNH3 / seconds);
    }
    /*if(!REDstable) {
      Serial.print("(RED:");
      Serial.print(abs(fltSumRED / seconds - curRED));
      Serial.println(")");
    }*/
    /*if(!OXstable) {
      Serial.print("(OX:");
      Serial.print(abs(fltSumOX / seconds - curOX));
      Serial.println(")");
    }*/

  } while (!NH3stable /*|| !REDstable*/ /*|| !OXstable*/);

  NH3baseR = fltSumNH3 / seconds;
  //REDbaseR = fltSumRED / seconds;
  //OXbaseR = fltSumOX / seconds;

  // Store new base resistance values in EEPROM
}

uint16_t getBaseResistance(channel_t channel) {
  switch (channel) {
    case CH_NH3: return NH3baseR;
    case CH_RED: return REDbaseR;
    case CH_OX: return OXbaseR;
  }
  
  return 0;
}


/**
   Calculates the current resistance ratio for the given channel.

   @param channel
          The channel to request resistance values from.
   @return The floating-point resistance ratio for the given channel.
*/
float getCurrentRatio(channel_t channel) {
  float baseResistance = (float) getBaseResistance(channel);
  float resistance = (float) getResistance(channel);

  return resistance / baseResistance * (32768.0 - baseResistance) / (32768.0 - resistance);
  
  return -1.0;
}

/**
   Measures the gas concentration in ppm for the specified gas.

   @param gas
          The gas to calculate the concentration for.
   @return The current concentration of the gas
           in parts per million (ppm).
*/
float measureMICS(gas_t gas) {
  float ratio;
  float c = 0;

  switch (gas) {
    case CO:
      ratio = getCurrentRatio(CH_RED);
      c = pow(ratio, -1.179) * 4.385;
      break;
    case NO2:
      ratio = getCurrentRatio(CH_OX);
      c = pow(ratio, 1.007) / 6.855;
      break;
    case NH3:
      ratio = getCurrentRatio(CH_NH3);
      c = pow(ratio, -1.67) / 1.47;
      break;
    case C3H8:
      ratio = getCurrentRatio(CH_NH3);
      c = pow(ratio, -2.518) * 570.164;
      break;
    case C4H10:
      ratio = getCurrentRatio(CH_NH3);
      c = pow(ratio, -2.138) * 398.107;
      break;
    case CH4:
      ratio = getCurrentRatio(CH_RED);
      c = pow(ratio, -4.363) * 630.957;
      break;
    case H2:
      ratio = getCurrentRatio(CH_RED);
      c = pow(ratio, -1.8) * 0.73;
      break;
    case C2H5OH:
      ratio = getCurrentRatio(CH_RED);
      c = pow(ratio, -1.552) * 1.622;
      break;
  }

  return isnan(c) ? -1 : c;
}
// example of values : nh3 = 0.7 ppm, co = 4.6 ppm and no2 = 0.17 ppm. I guess its ok based on the limits speculated on internet.

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

  calibrateMICS();
}

void loop() {
  mq131_read();
  float nh3_reading = measureMICS(NH3);
  // put your main code here, to run repeatedly:
  Serial.print("V_1:"); Serial.print(ads.computeVolts(ads.readADC_SingleEnded(1)));
  Serial.print(",MiCS:"); Serial.print(nh3_reading);
  Serial.print(",V_3:"); Serial.print(ads.computeVolts(ads.readADC_SingleEnded(3)));
  Serial.print(",O3:"); Serial.println(O3);
}

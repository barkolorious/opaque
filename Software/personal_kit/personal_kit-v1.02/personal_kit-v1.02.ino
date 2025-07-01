/*
 ██████████████████   ██████████████████  ███████████████████   ██████████████████  ██                ██  ██████████████████ 
██                ██ ██                ██                   ██ ██                ██ ██                ██ ██                ██
██                ██ ██                ██  ███████████████████ ██                ██ ██                ██ ███████████████████ 
██                ██ ██                ██ ██                ██ ██                ██ ██                ██ ██
 ██████████████████  ███████████████████   ██████████████████   ███████████████████  ██████████████████   ██████████████████ 
                     ██                                                          ██
                     ██                                                          ██            
*/
//////////////////////////////////// LIBS //////////////////////////////////////
#include <Wire.h>
#include <FS.h>
#include <SPI.h>
#include <SoftwareSerial.h>
//////////////// DS3231 ////////////////
#include <RTClib.h>
//////////////// ENS160 ////////////////
#include <SparkFun_ENS160.h>
//////////////// ENS210 ////////////////
#include <ens210.h>
/////////////// MPU6050 ////////////////
#include <MPU6050_light.h>
/////////////// SD Card ////////////////
#include <SD.h>
/////////////// ADS1115 ////////////////
#include <Adafruit_ADS1X15.h>
/////////////// PMS7003 ////////////////
#include <PMserial.h>
//////////////// NEO6M /////////////////
#include <TinyGPS++.h>
//////////////////////////////////// LIBS //////////////////////////////////////

#define MPU6050_DETECT_THRESHOLD 1.2
#define MQ131_ADS1115_PIN        0
#define MQ131_R_LOAD             22000.0
#define MICS6814_OX_ADS1115_PIN  1
#define MICS6814_NH3_ADS1115_PIN 2
#define MICS6814_CO_ADS1115_PIN  3
#define NEXTION_COMMAND_END      "\xFF\xFF\xFF"

//////////////////////////////////// PINS //////////////////////////////////////
/*        EMPTY         */ /* GPIO0 */
/*        EMPTY         */ /* GPIO1 (UART0 TX) -- CP2102 RX */
#define UART2_TX        2  /* GPIO2 (UART2 TX) -- NEO6M RX */
/*        EMPTY         */ /* GPIO3 (UART0 RX) -- CP2102 TX */
#define UART2_RX        4  /* GPIO4 (UART2 RX) -- NEO6M TX */
#define VSPI_CS         5  /* GPIO5 (VSPI CS)  -- SD Card CS */
/*      DO NOT USE      */ /* GPIO6 */
/*      DO NOT USE      */ /* GPIO7 */
/*      DO NOT USE      */ /* GPIO8 */
/*      DO NOT USE      */ /* GPIO9 */
/*      DO NOT USE      */ /* GPIO10 */
/*      DO NOT USE      */ /* GPIO11 */
/*        EMPTY         */ /* GPIO12 */
/*        EMPTY         */ /* GPIO13 */
/*        EMPTY         */ /* GPIO14 */
/*        EMPTY         */ /* GPIO15 */
#define UART1_RX        16 /* GPIO16 (UART1 RX) -- PMS7003 TX */
#define UART1_TX        17 /* GPIO17 (UART1 TX) -- PMS7003 RX */
#define VSPI_CLK        18 /* GPIO18 (VSPI CLK) */
#define VSPI_MISO       19 /* GPIO19 (VSPI MISO) */
#define I2C_SDA         21 /* GPIO21 (I2C SDA) */
#define I2C_SCL         22 /* GPIO22 (I2C SCL) */
#define VSPI_MOSI       23 /* GPIO23 (VSPI MOSI) */
#define UART3_RX        25 /* GPIO25 (SoftwareSerial [UART3] RX) -- Nextion TX */
#define UART3_TX        26 /* GPIO26 (SoftwareSerial [UART3] TX) -- Nextion RX */
/*        EMPTY         */ /* GPIO27 */
#define BTN_RECORD_PIN  32 /* GPIO32 -- Record Button */
#define BTN_BACKUP_PIN  33 /* GPIO33 -- Backup Button */
#define BAT_LEVEL_PIN   34 /* GPIO34 (Analog) -- Battery Charge Level */
#define NEXTION_INT_PIN 35 /* GPIO35 (Int) -- Nextion */
#define BAT_CHARGE_PIN  36 /* GPIO36 (Analog) -- Battery Is Charging?*/
/*        EMPTY         */ /* GPIO39 */
//////////////////////////////////// PINS //////////////////////////////////////

/////////////////////////////////// OBJECTS ////////////////////////////////////
//////////////// DS3231 ////////////////
RTC_DS3231 ds3231;
//////////////// ENS160 ////////////////
SparkFun_ENS160 ens160; 
//////////////// ENS210 ////////////////
ENS210 ens210;
/////////////// MPU6050 ////////////////
MPU6050 mpu6050(Wire);
/////////////// ADS1115 ////////////////
Adafruit_ADS1115 ads1115;
/////////////// PMS7003 ////////////////
SerialPM pms7003(PMS7003, UART1_RX, UART1_TX);
//////////////// NEO6M /////////////////
TinyGPSPlus neo6m;
HardwareSerial neo6m_serial(2);
/////////////// NEXTION ////////////////
SoftwareSerial nextion(UART3_RX, UART3_TX);
/////////////////////////////////// OBJECTS ////////////////////////////////////

////////////////////////////////// VARIABLES ///////////////////////////////////
/////////////// GENERAL ////////////////
double alarm_values[12] = {1e9, 1e9, 1e9, 1e9, 1e9, 1e9, 1e9, 1e9, 1e9, 1e9, 1e9, 1e9};
double measurements[12] = { -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1};
//////////////// DS3231 ////////////////
String ds3231_curr_date;
String ds3231_curr_time;
const char daysOfTheWeek[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char monthsOfTheYear[13][4] = {"N/A", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
//////////////// ENS160 ////////////////
uint16_t ens160_eCO2 = -1;
uint16_t ens160_tVOC = -1;
uint8_t  ens160_AQI = -1;
uint8_t  ens160_flag;
//////////////// ENS210 ////////////////
double ens210_temperature = -1;
double ens210_humidity = -1;
/////////////// MPU6050 ////////////////
uint8_t mpu6050_in_motion;
/////////////// SD Card ////////////////
uint8_t sd_card_type;
//////////////// MQ131 /////////////////
double mq131_O3;
/////////////// MICS6814 ///////////////
typedef enum mics6814_channel {CH_NH3, CH_RED, CH_OX} mics6814_channel_t;
typedef enum mics6814_gas {CO, NO2, NH3, C3H8, C4H10, CH4, H2, C2H5OH} mics6814_gas_t;
uint16_t mics6814_NH3_base_R;
uint16_t mics6814_RED_base_R;
uint16_t mics6814_OX_base_R;
double mics6814_NH3 = -1;
double mics6814_CO = -1;
double mics6814_NO2 = -1;
/////////////// PMS7003 ////////////////
uint16_t pms7003_PM0p1 = -1;
uint16_t pms7003_PM2p5 = -1;
uint16_t pms7003_PM10  = -1;
uint16_t pms7003_N0p3  = -1;
uint16_t pms7003_N0p5  = -1;
uint16_t pms7003_N1    = -1;
uint16_t pms7003_N2p5  = -1;
uint16_t pms7003_N5    = -1;
uint16_t pms7003_N10   = -1;
uint32_t pms7003_last_read = -1;
//////////////// NEO6M /////////////////
double neo6m_lat = -1;
double neo6m_lng = -1;
double neo6m_alt = -1;
/////////////// BUTTONS ////////////////
uint8_t button_record = 0;
uint8_t button_backup = 0;
/////////////// NEXTION ////////////////
const String nextion_pages[6] = {"home_page", "measurement_1", "measurement_2", "alarm_1", "alarm_2", "settings"};
const String nextion_measurement_objs[12] = {"measurement_1.temp_value.txt=\"", "measurement_1.humid_value.txt=\"", "measurement_1.co2_value.txt=\"", "measurement_1.tvoc_value.txt=\"", "measurement_1.aqi_value.txt=\"", "measurement_1.o3_value.txt=\"", "measurement_2.nh3_value.txt=\"", "measurement_2.no2_value.txt=\"", "measurement_2.co_value.txt=\"", "measurement_2.pm0p1_value.txt=\"", "measurement_2.pm2p5_value.txt=\"", "measurement_2.pm10_value.txt=\""};
////////////////////////////////// VARIABLES ///////////////////////////////////

int timer = 0;

void setup() {
  Wire.begin();
  Serial.begin(115200);

  delay(250);
  
  nextion_init();

  //ds3231_init();
  ens210_init();
  ens160_init();
  mpu6050_init();
  sd_card_init();
  ads1115_init();
  mq131_init();
  mics6814_init();
  pms7003_init();
  neo6m_init();
  buttons_init();

  Serial.println("started");
}

void loop() {
  if (millis() - timer > 5000) {
    timer = millis();
    ds3231_curr_date="Jun 14, Sat";
    ds3231_curr_time=millis();
    //ds3231_read();
    ens160_read();
    ens210_read();
    mpu6050_read();
    mq131_read();
    mics6814_read();
    pms7003_read();
    neo6m_read();
    nextion_update();
  }
  //buttons_read();
  nextion_read();

  delay(100);
}

//////////////////////////////////// DS3231 ////////////////////////////////////
void ds3231_init (void) {
  Serial.print(F("DS3231 starting... "));
  if (!ds3231.begin()) {
    nextion_display_error(F("failed"));
    for (;;) {}
  }
  Serial.println(F("done!"));

  Serial.print(F("DS3231 initializing... "));
  ds3231.adjust(DateTime(F(__DATE__), F(__TIME__)));
  Serial.println(F("done!"));
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


/////////////////////////////////// MPU6050 ////////////////////////////////////
void mpu6050_init (void) {
  Serial.print(F("MPU6050 starting... "));
  mpu6050.setAddress(0x69);
  if (mpu6050.begin()) {
    nextion_display_error(F("failed"));
    for (;;) {}
  }
  Serial.println(F("done!"));

  Serial.print(F("MPU6050 initializing... "));
  mpu6050.calcOffsets(true, true);
  Serial.println(F("done!"));
}

void mpu6050_read (void) {
  mpu6050.update();
  float x = mpu6050.getAccX();
  float y = mpu6050.getAccY();
  float z = mpu6050.getAccZ();
  float mag = sqrt(x * x + y * y + z * z);
  mpu6050_in_motion = (mag > MPU6050_DETECT_THRESHOLD);
  return;
}
/////////////////////////////////// MPU6050 ////////////////////////////////////

//////////////////////////////////// ENS160 ////////////////////////////////////
void ens160_init (void) {
  Serial.print(F("ENS160 starting... "));
  if (!ens160.begin()) {
		nextion_display_error(F("failed"));
    for (;;) {}
	}
  Serial.println(F("done!"));

	if( ens160.setOperatingMode(SFE_ENS160_RESET) ) {
    Serial.println(F("ENS160 initializing... done!"));
  }

	delay(100);

  ens160.setOperatingMode(SFE_ENS160_STANDARD);
  ens160_flag = ens160.getFlags();
  Serial.print(F("ENS160 Status Flag(0-Standard, 1-Warm up, 2-Initial Start Up): "));
	Serial.println(String(ens160_flag));
}

void ens160_read (void) {
  if (ens160.checkDataStatus())	{
		ens160_AQI  = ens160.getAQI();
    ens160_tVOC = ens160.getTVOC();
    ens160_eCO2 = ens160.getECO2();

    ens160_flag = ens160.getFlags();
	} 
  ens160_flag = -1;
}
//////////////////////////////////// ENS160 ////////////////////////////////////

//////////////////////////////////// ENS210 ////////////////////////////////////
void ens210_init (void) {
  Serial.print(F("ENS210 starting... "));
  ens210.begin();
  ens210.init();
  if (ens210.isConnected() == false) {
    nextion_display_error(F("failed"));
    for (;;) {}
  }
  Serial.println(F("done!"));

  Serial.print(F("ENS210 initializing..."));
  while (ens210.startContinuousMeasure() != RESULT_OK) {
    Serial.print(F("."));
    delay(ENS21X_SYSTEM_TIMING_BOOTING);
  }
  Serial.println(F(" done!"));
}

void ens210_read (void) {
  if (ens210.update() == RESULT_OK) {
    ens210_temperature = ens210.getTempCelsius();
    ens210_humidity    = ens210.getHumidityPercent();
  }
}
//////////////////////////////////// ENS210 ////////////////////////////////////

/////////////////////////////////// SD Card ////////////////////////////////////
void sd_card_init (void) {
  Serial.print(F("SD Card starting... "));
  SPI.begin(VSPI_CLK, VSPI_MISO, VSPI_MOSI, VSPI_CS);
  if (!SD.begin(5)) {
    nextion_display_error(F("failed"));
    for (;;) {}
  }
  Serial.println(F("done!"));

  Serial.print(F("SD Card initializing... "));
  sd_card_type = SD.cardType();
  if (sd_card_type == CARD_NONE) {
    nextion_display_error(F("failed"));
    for (;;) {}
  }
  Serial.println(F("done!"));
}

void sd_card_write_file (fs::FS &fs, const char *path, const char *message) {
  File file = fs.open(path, FILE_APPEND);

  if (!file) {
    nextion_display_error(F("ERR: SD Card writing failed"));
    return;
  }

  file.println(message);
  file.close();
}
/////////////////////////////////// SD Card ////////////////////////////////////

/////////////////////////////////// ADS1115 ////////////////////////////////////
void ads1115_init (void) {
  Serial.print(F("ADS1115 starting... "));
  if (!ads1115.begin()) {
    nextion_display_error(F("failed"));
    for (;;) {}
  }
  Serial.println(F("done!"));
}

float ads1115_read (uint8_t pin) {
  int16_t adc = ads1115.readADC_SingleEnded(0);
  return ads1115.computeVolts(adc);
}
/////////////////////////////////// ADS1115 ////////////////////////////////////

//////////////////////////////////// MQ131 /////////////////////////////////////
void mq131_init (void) {
  Serial.println(F("MQ131 starting... done!"));
}

void mq131_read (void) {
  double voltage = ads1115_read(MQ131_ADS1115_PIN);
  double R_s = (5.0 / voltage - 1.0) * MQ131_R_LOAD;
  double ratio = R_s / 15000 * mq131_get_environment_correction_ratio();

  mq131_O3 = (8.1399  * pow(ratio, 2.3297));
}

double mq131_get_environment_correction_ratio (void) {
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

/////////////////////////////////// MICS6814 ///////////////////////////////////
void mics6814_init (void) {
  Serial.print(F("MICS6814 starting... "));
  //mics6814_calibrate();
  Serial.println(F("done!"));
}

void mics6814_calibrate (void) {
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
  for (int i = 0; i < seconds; ++i) bufferNH3[i] = bufferRED[i] = bufferOX[i] = 0;

  do {
    // Wait a second
    delay(1000);
    // Read new resistances
    delay(50);
    
    unsigned long rs = 0;
    for (int i = 0; i < 3; i++) {
      delay(1);
      rs += ads1115.readADC_SingleEnded(MICS6814_NH3_ADS1115_PIN);
    }
    curNH3 = rs/3;

    rs = 0;
    delay(50);
    for (int i = 0; i < 3; i++) {
      delay(1);
      rs += ads1115.readADC_SingleEnded(MICS6814_CO_ADS1115_PIN);
    }
    curRED = rs/3;

    rs = 0;
    delay(50);
    for (int i = 0; i < 3; i++) {
      delay(1);
      rs += ads1115.readADC_SingleEnded(MICS6814_OX_ADS1115_PIN);
    }
    curOX = rs/3;

    // Update floating sum by subtracting value
    // about to be overwritten and adding the new value.
    fltSumNH3 = fltSumNH3 + curNH3 - bufferNH3[pntrNH3];
    fltSumRED = fltSumRED + curRED - bufferRED[pntrRED];
    fltSumOX  = fltSumOX  + curOX  - bufferOX[pntrOX];

    // Store new measurement in buffer
    bufferNH3[pntrNH3] = curNH3;
    bufferRED[pntrRED] = curRED;
    bufferOX[pntrOX]   = curOX;

    // Determine new state of flags
    NH3stable = abs(fltSumNH3 / seconds - curNH3) < delta;
    REDstable = abs(fltSumRED / seconds - curRED) < delta;
    OXstable  = abs(fltSumOX / seconds  - curOX)  < delta;

    // Advance buffer pointer
    pntrNH3 = (pntrNH3 + 1) % seconds ;
    pntrRED = (pntrRED + 1) % seconds;
    pntrOX = (pntrOX + 1) % seconds;

    //Mikä kestää?
    if(!NH3stable) {
      Serial.print("(NH3:");
      Serial.print(String(abs(fltSumNH3 / seconds - curNH3)));
      Serial.println(")");
    }
    if(!REDstable) {
      Serial.print("(RED:");
      Serial.print(String(abs(fltSumRED / seconds - curRED)));
      Serial.println(")");
    }
    if(!OXstable) {
      Serial.print("(OX:");
      Serial.print(String(abs(fltSumOX / seconds - curOX)));
      Serial.println(")");
    }

  } while (!NH3stable || !REDstable || !OXstable);

  mics6814_NH3_base_R = fltSumNH3 / seconds;
  mics6814_RED_base_R = fltSumRED / seconds;
  mics6814_OX_base_R  = fltSumOX  / seconds;
}

void mics6814_read (void) {
  mics6814_NH3 = mics6814_read_gas(NH3);
  mics6814_NO2 = mics6814_read_gas(NO2);
  mics6814_CO  = mics6814_read_gas(CO);
}

/**
   Requests the current resistance for a given channel
   from the sensor. The value is an ADC value between
   0 and 1024.

   @param channel
          The channel to read the base resistance from.
   @return The unsigned 16-bit base resistance
           of the selected channel.
*/
uint16_t mics6814_read_channel_R (mics6814_channel_t channel) {
  switch (channel) {
    case CH_NH3:
      return ads1115.readADC_SingleEnded(MICS6814_NH3_ADS1115_PIN);
    case CH_RED:
      return ads1115.readADC_SingleEnded(MICS6814_CO_ADS1115_PIN);
    case CH_OX:
      return ads1115.readADC_SingleEnded(MICS6814_OX_ADS1115_PIN);
  }

  return 0;
}

uint16_t mics6814_get_channel_base_R (mics6814_channel_t channel) {
  switch (channel) {
    case CH_NH3: return mics6814_NH3_base_R;
    case CH_RED: return mics6814_RED_base_R;
    case CH_OX:  return mics6814_OX_base_R;
  }
  return 0;
}

/**
   Calculates the current resistance ratio for the given channel.

   @param channel
          The channel to request resistance values from.
   @return The floating-point resistance ratio for the given channel.
*/
double mics6814_get_current_ratio(mics6814_channel_t channel) {
  double baseResistance = (double) mics6814_get_channel_base_R(channel);
  double resistance = (double) mics6814_read_channel_R(channel);

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
double mics6814_read_gas (mics6814_gas_t gas) {
  double ratio;
  double c = 0;

  switch (gas) {
    case CO:
      ratio = mics6814_get_current_ratio(CH_RED);
      c = pow(ratio, -1.179) * 4.385;
      break;
    case NO2:
      ratio = mics6814_get_current_ratio(CH_OX);
      c = pow(ratio, 1.007) / 6.855;
      break;
    case NH3:
      ratio = mics6814_get_current_ratio(CH_NH3);
      c = pow(ratio, -1.67) / 1.47;
      break;
    case C3H8:
      ratio = mics6814_get_current_ratio(CH_NH3);
      c = pow(ratio, -2.518) * 570.164;
      break;
    case C4H10:
      ratio = mics6814_get_current_ratio(CH_NH3);
      c = pow(ratio, -2.138) * 398.107;
      break;
    case CH4:
      ratio = mics6814_get_current_ratio(CH_RED);
      c = pow(ratio, -4.363) * 630.957;
      break;
    case H2:
      ratio = mics6814_get_current_ratio(CH_RED);
      c = pow(ratio, -1.8) * 0.73;
      break;
    case C2H5OH:
      ratio = mics6814_get_current_ratio(CH_RED);
      c = pow(ratio, -1.552) * 1.622;
      break;
  }

  return isnan(c) ? -1 : c;
}
/////////////////////////////////// MICS6814 ///////////////////////////////////

/////////////////////////////////// PMS7003 ////////////////////////////////////
void pms7003_init (void) {
  Serial.print(F("PMS7003 starting... "));
  pms7003.init();
  Serial.println(F("done!"));
}

void pms7003_read (void) {
  if (pms7003_last_read == -1) pms7003_last_read = millis();
  if (millis() - pms7003_last_read < 5000) return;
  pms7003_last_read = millis();

  pms7003.read();
  if (pms7003) {
    pms7003_PM0p1 = pms7003.pm01;
    pms7003_PM2p5 = pms7003.pm25;
    pms7003_PM10  = pms7003.pm10;
    pms7003_N0p3  = pms7003.n0p3;
    pms7003_N0p5  = pms7003.n0p5;
    pms7003_N1    = pms7003.n1p0;
    pms7003_N2p5  = pms7003.n2p5;
    pms7003_N5    = pms7003.n5p0;
    pms7003_N10   = pms7003.n10p0;
  } else {
    Serial.print(F("ERR: PMS7003 "));
    switch (pms7003.status) {
      case pms7003.OK: break;
      case pms7003.ERROR_TIMEOUT:     nextion_display_error(F(PMS_ERROR_TIMEOUT));     break;
      case pms7003.ERROR_MSG_UNKNOWN: nextion_display_error(F(PMS_ERROR_MSG_UNKNOWN)); break;
      case pms7003.ERROR_MSG_HEADER:  nextion_display_error(F(PMS_ERROR_MSG_HEADER));  break;
      case pms7003.ERROR_MSG_BODY:    nextion_display_error(F(PMS_ERROR_MSG_BODY));    break;
      case pms7003.ERROR_MSG_START:   nextion_display_error(F(PMS_ERROR_MSG_START));   break;
      case pms7003.ERROR_MSG_LENGTH:  nextion_display_error(F(PMS_ERROR_MSG_LENGTH));  break;
      case pms7003.ERROR_MSG_CKSUM:   nextion_display_error(F(PMS_ERROR_MSG_CKSUM));   break;
      case pms7003.ERROR_PMS_TYPE:    nextion_display_error(F(PMS_ERROR_PMS_TYPE));    break;
    }
  }
}
/////////////////////////////////// PMS7003 ////////////////////////////////////

//////////////////////////////////// NEO-6M ////////////////////////////////////
void neo6m_init (void) {
  Serial.print(F("NEO6M starting... "));
  neo6m_serial.begin(9600, SERIAL_8N1, UART2_RX, UART2_TX);
  if (!neo6m_serial) {
    nextion_display_error(F("failed"));
    for (;;) {}
  }
  Serial.println(F("done!"));
}

void neo6m_read (void) {
  unsigned long start = millis();

  while (millis() - start < 1000) {
    while (neo6m_serial.available() > 0) {
      neo6m.encode(neo6m_serial.read());
    }
    if (neo6m.location.isUpdated()) {
      neo6m_lat = neo6m.location.lat();
      neo6m_lng = neo6m.location.lng();
      neo6m_alt = neo6m.altitude.meters();
    }
  }
}
//////////////////////////////////// NEO-6M ////////////////////////////////////

/////////////////////////////////// BUTTONS ////////////////////////////////////
void buttons_init (void) {
  pinMode(BTN_BACKUP_PIN, INPUT);
  pinMode(BTN_RECORD_PIN, INPUT);
}

void buttons_read (void) {
  button_backup = digitalRead(BTN_BACKUP_PIN);
  button_record = digitalRead(BTN_RECORD_PIN);

  if (button_record) {
    String ens160_report     = "eCO2:[" + String(ens160_eCO2) + "],tVOC:[" + String(ens160_tVOC) + "],AQI:[" + String(ens160_AQI) + "],";
    String ens210_report     = "T:[" + String(ens210_temperature) + "],Hum:[" + String(ens210_humidity) + "],";
    String mpu6050_report    = "motion:[" + String(mpu6050_in_motion) + "],";
    String mq131_report      = "O3:[" + String(mq131_O3) + "],";
    String pms7003_PM_report = "PM:[" + String(pms7003_PM0p1) + "," + String(pms7003_PM2p5) + "," + String(pms7003_PM10) + "],";
    String pms7003_N_report  = "N:[" + String(pms7003_N0p3) + "," + String(pms7003_N0p3) + "," + String(pms7003_N1) + "," + String(pms7003_N2p5) + "," + String(pms7003_N5) + "," + String(pms7003_N10) + "],";
    String neo6m_report      = "LAT:[" + String(neo6m_lat) + "],LNG:[" + String(neo6m_lng) + "],ALT:[" + String(neo6m_alt) + "],";
    String mics6814_report   = "NH3:[" + String(mics6814_NH3) + "],NO2:[" + String(mics6814_NO2) + "],CO:[" + String(mics6814_CO) + "],";

    String all = ds3231_curr_date + "," + ds3231_curr_time + "," + ens160_report + ens210_report + mpu6050_report + mq131_report + mics6814_report + pms7003_PM_report + pms7003_N_report + neo6m_report;
    sd_card_write_file(SD, "/test.txt", all.c_str());
  }  
}
/////////////////////////////////// BUTTONS ////////////////////////////////////

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
  measurements[0]  = ens210_temperature;
  measurements[1]  = ens210_humidity;
  measurements[2]  = ens160_eCO2;
  measurements[3]  = ens160_tVOC;
  measurements[4]  = ens160_AQI;
  measurements[5]  = mq131_O3;
  measurements[6]  = mics6814_NH3;
  measurements[7]  = mics6814_NO2;
  measurements[8]  = mics6814_CO;
  measurements[9]  = pms7003_PM0p1;
  measurements[10] = pms7003_PM2p5;
  measurements[11] = pms7003_PM10;

  for (int i = 0; i < 12; i++) {
    nextion.print(nextion_measurement_objs[i] + measurements[i] + "\"" + NEXTION_COMMAND_END);
  }

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

  for (int i = 0; i < 6; i++) 
    nextion.print(nextion_pages[i] + ".clock.txt=\"" + ds3231_curr_time + "\"" + NEXTION_COMMAND_END);
  nextion.print("home_page.date.txt=\"" + ds3231_curr_date + "\"" + NEXTION_COMMAND_END);

  uint8_t bluetooth_connected = 0;
  for (int i = 0; i < 6; i++)
    nextion.print(nextion_pages[i] + ".bluetooth.pic=" + ((bluetooth_connected) ? 6 : 7) + NEXTION_COMMAND_END);
}

void nextion_read (void) {
  if (nextion.available()) {
    String nextion_input = "";
    nextion_input = nextion.readString();
    Serial.println(nextion_input);
    char mode = nextion_input[0];

    if (mode != 'A') {
      Serial.println("Nextion error");
      return;
    }

    int j = 1;
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
  }  
}
/////////////////////////////////// NEXTION ////////////////////////////////////
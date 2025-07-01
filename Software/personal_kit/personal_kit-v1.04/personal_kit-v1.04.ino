/*
 ██████████████████   ██████████████████  ███████████████████   ██████████████████  ██                ██  ██████████████████ 
██                ██ ██                ██                   ██ ██                ██ ██                ██ ██                ██
██                ██ ██                ██  ███████████████████ ██                ██ ██                ██ ███████████████████ 
██                ██ ██                ██ ██                ██ ██                ██ ██                ██ ██
 ██████████████████  ███████████████████   ██████████████████   ███████████████████  ██████████████████   ██████████████████ 
                     ██                                                          ██
                     ██                                                          ██            
*/
//////////////////////////////////// LIBS ////////////////////////////////////
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
/////////////// ESP-NOW ////////////////
#include <esp_now.h>
#include <WiFi.h>
///////////////////////////////////// LIBS /////////////////////////////////////

#define MPU6050_DETECT_THRESHOLD 1.2
#define MQ131_ADS1115_PIN        0
#define MQ131_R_LOAD             22000.0
#define MICS6814_OX_ADS1115_PIN  1
#define MICS6814_NH3_ADS1115_PIN 2
#define MICS6814_CO_ADS1115_PIN  3
#define NEXTION_COMMAND_END      "\xFF\xFF\xFF"

///////////////////////////////////// PINS /////////////////////////////////////
/*        EMPTY         */ /* GPIO0 */
/*        EMPTY         */ /* GPIO1 (UART0 TX) -- CP2102 RX */
#define UART2_TX        2  /* GPIO2 (UART2 TX) -- NEO6M RX (WiFi) */
/*        EMPTY         */ /* GPIO3 (UART0 RX) -- CP2102 TX */
#define UART2_RX        4  /* GPIO4 (UART2 RX) -- NEO6M TX (WiFi) */
#define VSPI_CS         5  /* GPIO5 (VSPI CS)  -- SD Card CS */
/*      DO NOT USE      */ /* GPIO6 */
/*      DO NOT USE      */ /* GPIO7 */
/*      DO NOT USE      */ /* GPIO8 */
/*      DO NOT USE      */ /* GPIO9 */
/*      DO NOT USE      */ /* GPIO10 */
/*      DO NOT USE      */ /* GPIO11 */
/*        EMPTY         */ /* GPIO12 (WiFi) */
/*        EMPTY         */ /* GPIO13 (WiFi) */
/*        EMPTY         */ /* GPIO14 (WiFi) */
/*        EMPTY         */ /* GPIO15 (WiFi) */
#define UART1_RX        16 /* GPIO16 (UART1 RX) -- PMS7003 TX */
#define UART1_TX        17 /* GPIO17 (UART1 TX) -- PMS7003 RX */
#define VSPI_CLK        18 /* GPIO18 (VSPI CLK) */
#define VSPI_MISO       19 /* GPIO19 (VSPI MISO) */
#define I2C_SDA         21 /* GPIO21 (I2C SDA) */
#define I2C_SCL         22 /* GPIO22 (I2C SCL) */
#define VSPI_MOSI       23 /* GPIO23 (VSPI MOSI) */
//#define UART3_RX        25 /* GPIO25 (SoftwareSerial [UART3] RX) -- Nextion TX (WiFi) */
#define UART3_TX        26 /* GPIO26 (SoftwareSerial [UART3] TX) -- Nextion RX (WiFi) */
/*        EMPTY         */ /* GPIO27 (WiFi) */
#define BTN_RECORD_PIN  32 /* GPIO32 -- Record Button */
#define BTN_BACKUP_PIN  33 /* GPIO33 -- Backup Button */
#define BAT_LEVEL_PIN   34 /* GPIO34 (Analog) -- Battery Charge Level */
#define UART3_RX 35 /* GPIO35 (Int) -- Nextion */
#define BAT_CHARGE_PIN  36 /* GPIO36 (Analog) -- Battery Is Charging?*/
/*        EMPTY         */ /* GPIO39 */
///////////////////////////////////// PINS /////////////////////////////////////

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
uint32_t measurement_timer = 0;
uint32_t espnow_timer = 0;
float alarm_values[12] = {1e9, 1e9, 1e9, 1e9, 1e9, 1e9, 1e9, 1e9, 1e9, 1e9, 1e9, 1e9};
float measurements[12] = { -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1};
bool is_alarm = false;
uint8_t alarm_reason = -1;
uint32_t last_alarm_time = 0;
//////////////// DS3231 ////////////////
String ds3231_curr_date;
String ds3231_curr_time;
const char daysOfTheWeek[7][4] = {"Paz", "Pzt", "Sal", "Car", "Per", "Cum", "Cts"};
const char monthsOfTheYear[13][4] = {"N/A", "Oca", "Sub", "Mar", "Nis", "May", "Haz", "Tem", "Agu", "Eyl", "Eki", "Kas", "Ara"};
//////////////// ENS160 ////////////////
uint16_t ens160_eCO2 = -1;
uint16_t ens160_tVOC = -1;
uint8_t  ens160_AQI = -1;
uint8_t  ens160_flag;
//////////////// ENS210 ////////////////
float ens210_temperature = -1;
float ens210_humidity = -1;
/////////////// MPU6050 ////////////////
uint8_t mpu6050_in_motion;
/////////////// SD Card ////////////////
uint8_t sd_card_type;
//////////////// MQ131 /////////////////
float mq131_O3;
/////////////// MICS6814 ///////////////
typedef enum mics6814_channel {CH_NH3, CH_RED, CH_OX} mics6814_channel_t;
typedef enum mics6814_gas {CO, NO2, NH3, C3H8, C4H10, CH4, H2, C2H5OH} mics6814_gas_t;
uint16_t mics6814_NH3_base_R;
uint16_t mics6814_RED_base_R;
uint16_t mics6814_OX_base_R;
float mics6814_NH3 = -1;
float mics6814_CO = -1;
float mics6814_NO2 = -1;
/////////////// PMS7003 ////////////////
uint16_t pms7003_PM1   = -1;
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
float neo6m_lat = -1;
float neo6m_lng = -1;
float neo6m_alt = -1;
/////////////// BUTTONS ////////////////
uint8_t button_record = 0;
uint8_t button_backup = 0;
/////////////// NEXTION ////////////////
const String nextion_pages[6] = {"home_page", "measurement_1", "measurement_2", "alarm_1", "alarm_2", "settings"};
const String nextion_gas_names[12] = {"Temp", "Humid", "CO2", "tVOC", "AQI", "O3", "NH3", "NO2", "CO", "PM1", "PM2.5", "PM10"};
const String nextion_measurement_objs[12] = {"measurement_1.temp_value.txt=\"",  "measurement_1.humid_value.txt=\"", 
                                             "measurement_1.co2_value.txt=\"",   "measurement_1.tvoc_value.txt=\"", 
                                             "measurement_1.aqi_value.txt=\"",   "measurement_1.o3_value.txt=\"", 
                                             "measurement_2.nh3_value.txt=\"",   "measurement_2.no2_value.txt=\"", 
                                             "measurement_2.co_value.txt=\"",    "measurement_2.pm1_value.txt=\"", 
                                             "measurement_2.pm2p5_value.txt=\"", "measurement_2.pm10_value.txt=\""};
/////////////// ESP-NOW ////////////////
typedef struct struct_message { float longitude, latitude; float measurements[12]; bool alert; } espnow_message_t;
espnow_message_t espnow_data;
esp_now_peer_info_t espnow_info;
uint8_t espnow_prmd_addr[] = {0x5C, 0xCF, 0x7F, 0x16, 0x75, 0x9A};
uint8_t espnow_reciever_addr[] = {0xE4, 0x65, 0xB8, 0xD8, 0x93, 0x04};
////////////////////////////////// VARIABLES ///////////////////////////////////

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
  espnow_init();

  Serial.println("started");
}

void loop() {
  if (millis() - measurement_timer > 5000) {
    measurement_timer = millis();
    //ds3231_curr_date="Jun 14, Sat";
    //ds3231_curr_time=millis();
    //ds3231_read();
    ens160_read();
    ens210_read();
    mpu6050_read();
    mq131_read();
    mics6814_read();
    pms7003_read();
    neo6m_read();

    update_measurements();
    check_alarms();

    nextion_update();
    espnow_update();
  }

  if (millis() - espnow_timer > 2500) {
    espnow_timer = millis();
    espnow_update();
  }

  buttons_read();
  nextion_read();

  uint32_t wait_timer = millis();
  while (millis() - wait_timer < 100) ;
}

void update_measurements (void) {
  measurements[0]  = ens210_temperature;
  measurements[1]  = ens210_humidity;
  measurements[2]  = ens160_eCO2;
  measurements[3]  = ens160_tVOC;
  measurements[4]  = ens160_AQI;
  measurements[5]  = mq131_O3;
  measurements[6]  = mics6814_NH3;
  measurements[7]  = mics6814_NO2;
  measurements[8]  = mics6814_CO;
  measurements[9]  = pms7003_PM1;
  measurements[10] = pms7003_PM2p5;
  measurements[11] = pms7003_PM10;
}

void check_alarms (void) {
  is_alarm = false;
  alarm_reason = -1;
  for (int i = 0; i < 12; i++) {
    if (!std::isnormal(measurements[i])) continue;
    if (measurements[i] > alarm_values[i]) {
      handle_alarm(i);
      return;
    }
  }
}

void handle_alarm (uint8_t gas) {
  is_alarm = true;
  alarm_reason = gas;
  last_alarm_time = millis();

  Serial.print("reason " + nextion_gas_names[alarm_reason]);
  Serial.print(", alarm ");
  Serial.print(alarm_values[alarm_reason]);
  Serial.print(", measure ");
  Serial.println(measurements[alarm_reason]);


  if (nextion) {
    nextion.print("alarm.reason.txt=\"" + nextion_gas_names[alarm_reason] + "\"\xFF\xFF\xFF");
    nextion.print("alarm.alarm_value.txt=\"" + String(alarm_values[alarm_reason]) + "\"\xFF\xFF\xFF");
    nextion.print("alarm.measurement.txt=\"" + String(measurements[alarm_reason]) + "\"\xFF\xFF\xFF");
    nextion.print("page alarm\xFF\xFF\xFF");
  }

  espnow_update();
}

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
    String pms7003_PM_report = "PM:[" + String(pms7003_PM1) + "," + String(pms7003_PM2p5) + "," + String(pms7003_PM10) + "],";
    String pms7003_N_report  = "N:[" + String(pms7003_N0p3) + "," + String(pms7003_N0p3) + "," + String(pms7003_N1) + "," + String(pms7003_N2p5) + "," + String(pms7003_N5) + "," + String(pms7003_N10) + "],";
    String neo6m_report      = "LAT:[" + String(neo6m_lat) + "],LNG:[" + String(neo6m_lng) + "],ALT:[" + String(neo6m_alt) + "],";
    String mics6814_report   = "NH3:[" + String(mics6814_NH3) + "],NO2:[" + String(mics6814_NO2) + "],CO:[" + String(mics6814_CO) + "],";

    String all = ds3231_curr_date + "," + ds3231_curr_time + "," + ens160_report + ens210_report + mpu6050_report + mq131_report + mics6814_report + pms7003_PM_report + pms7003_N_report + neo6m_report;
    sd_card_write_file(SD, "/test.txt", all.c_str());
  }  
}
/////////////////////////////////// BUTTONS ////////////////////////////////////
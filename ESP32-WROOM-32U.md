![[Pasted image 20241114225116.png]]
![[Pasted image 20241116133241.png]]

- ESP32 + SSD1306: 
	- https://lastminuteengineers.com/oled-display-esp32-tutorial/
- ESP32 + SD Card ( + [Data logging](https://randomnerdtutorials.com/esp32-microsd-card-arduino/#webserversdcard)): 
	- https://www.electronicwings.com/esp32/microsd-card-interfacing-with-esp32
	- https://randomnerdtutorials.com/esp32-microsd-card-arduino/
- ESP32 + SSD1306 + DS3231:
	- https://microcontrollerslab.com/esp32-ds3231-real-time-clock-rtc-oled/
- ESP32 + MPU6050:
	- https://randomnerdtutorials.com/esp32-mpu-6050-accelerometer-gyroscope-arduino/
- ESP32 Analog Input:
	- https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
- ESP32 + Firebase:
	- https://randomnerdtutorials.com/esp32-firebase-web-app/
- ESP32 + SIM800C:
	- https://community.thinger.io/t/esp32-sim800c/676/3
- ESP32 + SIM800L:
	- https://randomnerdtutorials.com/esp32-cloud-mqtt-broker-sim800l/
	- https://circuitdigest.com/microcontroller-projects/interfacing-sim800l-module-with-esp32
- ESP32 + NEO-6M:
	- https://randomnerdtutorials.com/esp32-neo-6m-gps-module-arduino/
- ESP32 I<sup>2</sup>C Pull-up: 
	- https://electronics.stackexchange.com/questions/602404/esp32-i2c-circuit
	> Most sensors we use in our projects are breakout boards that already have the resistors built-in. So, usually, when you’re dealing with this type of electronics components you don’t need to worry about this.
	> - https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/
- ESP32 + PM7003:
	- https://registry.platformio.org/libraries/avaldebe/PMSerial
	- https://github.com/avaldebe/PMserial
	- https://github.com/Ptelka/PMS7003
- MICS6814
	- https://forum.arduino.cc/t/cjmcu-6814-readings-into-ppm/1257495/5
	- https://store.siqma.com/mics-6814-air-quality-sensor-module.html
		- https://www.youtube.com/watch?v=ByzoAjvoX58
			- https://github.com/noorkhokhar99/MICS6814/tree/master
	- https://github.com/Seeed-Studio/Mutichannel_Gas_Sensor
	- maybe you forgot pullup-resistors? Do you use the MiCS-6814 on the purple adapter board CJMCU6814? Then you may add an eg 47k ohm resistor from the analog Input (A1 in your sketch) to Vcc (5V or 3,3 V, depends on your arduino). Then you should get other measurment than the 0-values.  BTW: the sensor needs some time to heat up. And I'm not sure about it's stability over time. I've no experience with it yet.
	- https://kstobbe.dk/2019/02/16/esp32-pms5003-bme280-mics6814-sensor-build/
	- https://forum.arduino.cc/t/cjmcu-6814-adapter-board-with-mics-6814-co-nh3-no2-sensor/595612/14
	- https://forum.arduino.cc/t/mics-6814-cjmcu-6814-gas-sensor-reading-conversion/992844
	- https://forum.arduino.cc/t/mics-6814-integrated-on-a-board-with-esp-wroom-32d-v_rs-reads-2-98-v-of-3-3/699348
	- https://forum.arduino.cc/t/cjmcu-6814-readings-into-ppm/1348009
	- ![[Pasted image 20250128162927.png]]
- MQ-131
	- https://github.com/ostaquet/Arduino-MQ131-driver
	- https://www.graphreader.com/v2
	- https://www.winsen-sensor.com/d/files/manual/mq131-h.pdf
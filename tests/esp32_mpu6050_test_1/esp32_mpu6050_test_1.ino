#include <Wire.h>
#include <MPU6050_light.h>

#define MPU6050_DETECT_THRESHOLD 1.2

MPU6050 mpu(Wire);

void setup() {
  Serial.begin(115200);
  Wire.begin();

  if (mpu.begin()) {
    Serial.println(F("initialization failed! (MPU6050)"));
    Serial.flush();
    return;
  }

  Serial.println(F("calibrating MPU6050"));
  delay(1000);
  mpu.calcOffsets(true,true);
  Serial.println(F("calibration done! (MPU6050)"));
}

void loop() {
  uint8_t is_in_motion = mpu6050_is_in_motion();
  Serial.print("a:");
  Serial.println(is_in_motion);

  delay(50);
}

uint8_t mpu6050_is_in_motion (void) {
  mpu.update();
  float x = mpu.getAccX();
  float y = mpu.getAccY();
  float z = mpu.getAccZ();
  float mag = sqrt(x * x + y * y + z * z);
  return (mag > MPU6050_DETECT_THRESHOLD);
}
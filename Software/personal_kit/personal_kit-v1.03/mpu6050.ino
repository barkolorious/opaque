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
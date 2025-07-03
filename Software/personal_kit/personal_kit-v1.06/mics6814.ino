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
float mics6814_get_current_ratio(mics6814_channel_t channel) {
  float baseResistance = (float) mics6814_get_channel_base_R(channel);
  float resistance = (float) mics6814_read_channel_R(channel);

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
float mics6814_read_gas (mics6814_gas_t gas) {
  float ratio;
  float c = 0;

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
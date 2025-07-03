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
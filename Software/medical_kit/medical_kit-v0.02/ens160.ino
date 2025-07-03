//////////////////////////////////// ENS160 ////////////////////////////////////
void ens160_init (void) {
  ssd1306_new_screen();
  ssd1306_print(F("ENS160 starting... "));
  if (!ens160.begin()) {
		ssd1306_println(F("failed"));
    for (;;) {}
	}
  ssd1306_println(F("done!"));

	if( ens160.setOperatingMode(SFE_ENS160_RESET) ) {
    ssd1306_println(F("ENS160 initializing... done!"));
  }

	delay(100);

  ens160.setOperatingMode(SFE_ENS160_STANDARD);
  ens160_flag = ens160.getFlags();
  ssd1306_print(F("ENS160 Status Flag(0-Standard, 1-Warm up, 2-Initial Start Up): "));
	ssd1306_println(String(ens160_flag));
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
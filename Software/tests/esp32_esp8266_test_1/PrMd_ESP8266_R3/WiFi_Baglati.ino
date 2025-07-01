

void Check_WiFi_and_Connect() {
  
       if (WiFi.status() != WL_CONNECTED) {
        
                  digitalWrite(bagli, HIGH);
                  delay(4);
                  
                  WiFi.disconnect();
                  delay(20);
                  
                  WiFi.begin("AirQ-M");
                  delay(20);
              
                  while (WiFi.status() != WL_CONNECTED) { delay(100); }
              
                  Udp.begin(UDPPort);
        }

        if (WiFi.status() == WL_CONNECTED) { digitalWrite(bagli, LOW); }

}

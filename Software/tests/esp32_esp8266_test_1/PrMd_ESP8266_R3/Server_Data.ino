

void Data_From_Server() {

  unsigned long tNow;
  tNow = millis();

  String esleme = "PrMd";
  esleme.toCharArray(result, esleme.length() + 1);
  Udp.beginPacket(APlocal_IP, UDPPort);
  Udp.write(result);
  Udp.endPacket();

  while (1) {
    
    int packetSize = Udp.parsePacket();
          
          if (packetSize) {
                int len = Udp.read(packetBuffer, 255);
                
                if (len > 0) { packetBuffer[len] = 0; }
          
                uyari = packetBuffer[0];
                
                break;
          }
      
          if ((millis() - tNow) > 1000) { break; }
  }
  
}

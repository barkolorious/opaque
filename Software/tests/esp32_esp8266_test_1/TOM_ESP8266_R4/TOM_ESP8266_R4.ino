//*****************************************************************************************************
//*****************************************************************************************************
/*
      AirQ-M PROJESİ TÖM ESP8266-01 YAZILIMI
      
      Barkın Özsoy Ocak - 2023
*/
//*****************************************************************************************************
//*****************************************************************************************************
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <string.h>
#define     LED0        2         // WIFI Module LED

/* WiFi Ayarları */

  #define     MAXSC     6                 // Maksimum Bağlantı sayısı
    
  IPAddress APlocal_IP(192, 168, 4, 1);
  IPAddress APgateway(192, 168, 4, 1);
  IPAddress APsubnet(255, 255, 255, 0);
  unsigned int UDPPort = 2390;            // lokal port
  WiFiUDP Udp;
  
  char packetBuffer[255]; 
  char result[20];
  volatile byte veriKontrol = 0;
  String uyari = "N";


void SetWifi();
void HandleClients();

//*****************************************************************************************************
//*****************************************************************************************************
//                            SETUP  
//*****************************************************************************************************
//*****************************************************************************************************

void setup() {
 Serial.begin(115200);
 //Serial.begin(9600);
 uyari.reserve(1);
 SetWifi();
 delay(100);
}

//*****************************************************************************************************
//*****************************************************************************************************
//                VOID LOOP
//*****************************************************************************************************
//*****************************************************************************************************

void loop() {

  HandleClients();
  delay(100);
}

//*****************************************************************************************************
//*****************************************************************************************************
//       ALARM KONTROL
//*****************************************************************************************************
//*****************************************************************************************************

void serialEvent() { 
   uyari = "";
    while (Serial.available()) {
          char inChar = (char)Serial.read();
          uyari += inChar;
          veriKontrol = 1; 
      }
}

//*****************************************************************************************************
//*****************************************************************************************************
//       WiFi BÖLGESİ (AP UDP)
//*****************************************************************************************************
//*****************************************************************************************************

void HandleClients(){
  
  unsigned long tNow;
  
  int packetSize = Udp.parsePacket();
  
  if (packetSize) {
        int len = Udp.read(packetBuffer, 255);
        if (len > 0) { packetBuffer[len] = 0; }
        //Serial.println(packetBuffer);
        Serial.print("P");
        //Serial.println("");
        
    }
    else
    {
          SetWifi();
          Serial.print("S"); 
          //Serial.println("");
          delay(25);
    }

    //tNow=millis();
    String temp = uyari;
    
    temp.toCharArray(result,temp.length()+1);
    
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(result);
    Udp.endPacket();
    
}

void SetWifi(){
  
//  WiFi.disconnect();
//  delay(10);
  WiFi.mode(WIFI_AP_STA);
  delay(50);
  //Serial.println("WIFI Mode : AccessPoint Station");

  //char*  ssid = "AirQ-M" ;              
  //char*  password = "";     

  String  ssid = "AirQ-M" ;              
  String  password = "";     
                
  
  WiFi.softAPConfig(APlocal_IP, APgateway, APsubnet);                   
  WiFi.softAP(ssid, password, 1 ,0, MAXSC);                                                      
  //Serial.println("WIFI < " + String(ssid) + " > ... Started");

  delay(50);

  IPAddress IP = WiFi.softAPIP();
  delay(50); 
  //Serial.print("AccessPoint IP : ");
  //Serial.println(IP);

  Udp.begin(UDPPort);
  //Serial.println("Server Started");
}

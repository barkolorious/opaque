//*****************************************************************************************************
//*****************************************************************************************************
/*
      AirQ-M PROJESİ PrMd ESP8266-01 YAZILIMI

      Barkın Özsoy Ocak - 2023
*/
//*****************************************************************************************************
//*****************************************************************************************************
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#define   LED0    2


//byte      yedek = 0;
volatile int        bagli = 1;
// GPIO 2 kütüphaneler kulanıyor
volatile int        tetik = 3;

/*
      GPIO 0 - Program Yüklemede kulanılıyor GND'ye çekmek sıkıntılı
      GPIO 1 - TX
      GPIO 2 - Wifi Programı kulanıyor, Program içinde kulanılamaz
      GPIO 3 - RX
*/


char*       ssid;
char*       password;
const String  Devicename = "PrMd";

IPAddress APlocal_IP(192, 168, 4, 1); // 192.168.0.0 – 192.168.255.255 arasında secilebilir ( C Sınıfı Ağlar)
IPAddress APgateway(192, 168, 4, 1); //
IPAddress APsubnet(255, 255, 255, 0); // C sınıfı ağların alt ağ maskesi ön tanımlı olarak 255.255.255.0 olur.

unsigned int UDPPort = 2390;

WiFiUDP Udp;

char result[20];
char packetBuffer[255];
unsigned long giris;

String uyari = "N";



void Check_WiFi_and_Connect();
void Data_From_Server();

//*****************************************************************************************************
//*****************************************************************************************************
//                            SETUP
//*****************************************************************************************************
//*****************************************************************************************************

void setup() {

  //Serial.begin(115200);

  pinMode(tetik, OUTPUT);

  pinMode(bagli, OUTPUT);
  digitalWrite(bagli, HIGH);


      digitalWrite(tetik, HIGH);
      delay(5000);
      digitalWrite(tetik, LOW); // SetUp komtrol noktası


  Check_WiFi_and_Connect();

  giris = millis();
}

//*****************************************************************************************************
//*****************************************************************************************************
//                VOID LOOP
//*****************************************************************************************************
//*****************************************************************************************************

void loop() {

  Data_From_Server();
  
  if (WiFi.status() != WL_CONNECTED) { Check_WiFi_and_Connect();}

  if ((millis() - giris ) > 5000  && uyari != "A" && digitalRead(tetik) ) {
    digitalWrite(tetik, LOW);
    delay(25);
  }

  if ( uyari == "A" && !digitalRead(tetik)) {
    giris = millis();
    digitalWrite(tetik, HIGH);
  }
  
  delay(10);
}

#include <SPI.h>
#include <SD.h>

File myFile;

const int buttonPin = 2;
const int CS = 10;

void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPin, INPUT);
  Serial.begin(9600);

  Serial.print("Initializing SD card...");

  if (!SD.begin(CS)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  myFile = SD.open("test.csv", FILE_WRITE);

  if (myFile) {
    Serial.println("test.csv opened...");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.csv");
    return;
  }

  myFile.close();
}

void loop() {
  // put your main code here, to run repeatedly:
  int button = digitalRead(buttonPin);
  myFile = SD.open("test.csv", FILE_WRITE);
  if (myFile) {
    myFile.print(millis());
    myFile.print(",");
    myFile.println(button);
    myFile.close();
    Serial.println(button); 
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  delay(100);
}

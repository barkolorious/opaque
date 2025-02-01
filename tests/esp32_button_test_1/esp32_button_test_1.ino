int dugme1 = 34, dugme2 = 35;

void setup () {
  Serial.begin(115200);

  pinMode(dugme1, INPUT);
  pinMode(dugme2, INPUT);

  Serial.println("started...");
}


void loop () {
  int deger1 = digitalRead(dugme1);
  int deger2 = digitalRead(dugme2);

  Serial.print("dgr_1:");Serial.print(deger1);
  Serial.print(",dgr_2:");Serial.println(deger2);
}
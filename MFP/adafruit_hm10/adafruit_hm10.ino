#include <bluefruit.h>
#include <SoftwareSerial.h>

const int RX = 14;
const int TX = 15;

SoftwareSerial hmSerial(RX, TX); //(RX, TX)
void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  hmSerial.begin(9600);
  // Wait for Serial1 to initialize
  while (!hmSerial) {
    delay(10);
  }
}
char c = ' ';
void loop() {
  // put your main code here, to run repeatedly:
  if(hmSerial.available())
  {
    c = hmSerial.read();
    Serial.write(c);
    Serial.println("message received");
  }

  if(Serial.available()) {
    c = Serial.read();
    Serial.println("message sent");
    hmSerial.write(c);
  }
}

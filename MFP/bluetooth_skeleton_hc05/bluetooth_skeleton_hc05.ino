#include <SoftwareSerial.h>
SoftwareSerial Bluetooth(9, 10); //TX || RX

void setup() {
  // put your setup code here, to run once:
  Bluetooth.begin(9600);
  Serial.begin(9600);
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    Bluetooth.write(Serial.read());
  }
    
  
  if (Bluetooth.available()) {
    Serial.write(Bluetooth.read());
  } 
}

#include <SoftwareSerial.h>
const int RX = 8;
const int TX = 9;
SoftwareSerial bleSerial = SoftwareSerial(RX, TX); // RX, TX

void setup() {
  Serial.begin(9600);
  bleSerial.begin(9600);
}

char c=' ';
boolean NL = true;
void loop() {
  if(bleSerial.available())
  {
    c = bleSerial.read();
    Serial.write(c);
  }

  if(Serial.available()) {
    c = Serial.read();

    bleSerial.write(c);
  }
  // // Echo the user input to the main window. 
  // // If there is a new line print the ">" character.
  // if (NL) { Serial.print("\r\n>");  NL = false; }
  // Serial.write(c);
  // if (c==10) { NL = true; }
  // }
}

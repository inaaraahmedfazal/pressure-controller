#include <SoftwareSerial.h>
const int RX = 8; //green cord
const int TX = 9; //orange cord
//const int digitest = 7;
SoftwareSerial ccSerial = SoftwareSerial(RX, TX); // RX, TX

void setup() {
  Serial.begin(9600);
  ccSerial.begin(9600);
}

char c=' ';
void loop() {
  //Reads
  if(ccSerial.available())
  {
    c = ccSerial.read();
    Serial.write(c);
  }

  if(Serial.available()) {
    c = Serial.read();

    ccSerial.write(c);
  }

  //Writes (comment out if just reading)
  // ccSerial.write("hello");
  // delay(500);
  // // Echo the user input to the main window. 
  // // If there is a new line print the ">" character.
  // if (NL) { Serial.print("\r\n>");  NL = false; }
  // Serial.write(c);
  // if (c==10) { NL = true; }
  // }
}

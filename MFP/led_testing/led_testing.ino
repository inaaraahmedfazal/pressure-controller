// const int red = 8;
// const int green = 9;
#include <bluefruit.h>
const int button_mute = 1;
byte button_mute_last_state = LOW;

void setup() {
  // put your setup code here, to run once:
  // pinMode(red, OUTPUT);
  // pinMode(green, OUTPUT);
  Serial.begin(9600);
  pinMode(button_mute, INPUT);
}

void loop() {
  // // put your main code here, to run repeatedly:
  // //RED
  // analogWrite(red, 255);
  // analogWrite(green, 0);
  // delay(5000);

  // //YELLOW
  // analogWrite(red, 255);
  // analogWrite(green, 100);
  // delay(5000);

  // //GREEN
  // analogWrite(red, 0);
  // analogWrite(green, 255);
  // delay(5000);

  byte button_mute_state = digitalRead(button_mute);
  if (button_mute_state != button_mute_last_state) {
    button_mute_last_state = button_mute_state;
    if (button_mute_state == HIGH) {
      digitalWrite(red_q2, 0);
      digitalWrite(green_q2, 255);
    }
    if(button_mute_state == LOW) {
      Serial.println("button mute state is LOW");
    }
  }
}

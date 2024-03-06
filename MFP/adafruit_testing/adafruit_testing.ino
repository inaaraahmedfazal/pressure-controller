// LED PIN DEFINITIONS --------------------------------------------------
 
// Power LED Pinout
const int red_power = 22;
const int green_power = 2;
 
// Quadrant 1 LED Pinout
const int red_q1 = 9;
const int green_q1 = 6;
 
// Quadrant 2 LED Pinout
const int red_q2 = 5;
const int green_q2 = 23;
 
// Quadrant 3 LED Pinout
const int red_q3 = 13;
const int green_q3 = 12;
 
// Quadrant 4 LED Pinout
const int red_q4 = 11;
const int green_q4 = 10;
 
// BUTTON PIN DEFINITIONS -----------------------------------------------
 
// Mute Button Pinout
//const int button_mute = ;
//byte button_mute_last_state = LOW;
 
// SLIDE SWITCH PIN DEFINITIONS -----------------------------------------
 
// Slide Switch Pinout
//const int switch_mode = ;
//int switch_mode_state;
 
// BUZZER PIN DEFINITIONS -----------------------------------------------
 
// Buzzer Pinout
//const int buzzer = ;
 
//bool once = false;
 
void setup() {
  pinMode(red_power, OUTPUT);
  pinMode(green_power, OUTPUT);
//  pinMode(red_q1, OUTPUT);
//  pinMode(green_q1, OUTPUT);
//
//  pinMode(red_q2, OUTPUT);
//  pinMode(green_q2, OUTPUT);
//  
//  pinMode(red_q3, OUTPUT);
//  pinMode(green_q3, OUTPUT);
//
//  pinMode(red_q4, OUTPUT);
//  pinMode(green_q4, OUTPUT);
 
//  pinMode(switch_mode, INPUT);
//  pinMode(buzzer, OUTPUT);
}
 
void loop() {
  // Flash LEDs Green
  digitalWrite(red_power, 0);
  analogWrite(green_power, 255);
//  digitalWrite(red_q1, 0);
//  digitalWrite(green_q1, 255);
//
//  digitalWrite(red_q2, 0);
//  digitalWrite(green_q2, 255);
//  
//  digitalWrite(red_q3, 0);
//  digitalWrite(green_q3, 255);
//  
//  digitalWrite(red_q3, 0);
//  digitalWrite(green_q3, 255);
 
  delay(500);
  // Flash LEDs Yellow
  digitalWrite(red_power, 255);
  analogWrite(green_power, 70);
//  digitalWrite(red_q1, 255);
//  digitalWrite(green_q1, 70);
//
//  digitalWrite(red_q2, 255);
//  digitalWrite(green_q2, 70);
//  
//  digitalWrite(red_q3, 255);
//  digitalWrite(green_q3, 70);
//  
//  digitalWrite(red_q3, 255);
//  digitalWrite(green_q3, 70);
 
  delay(500);
 
  // Flash LEDs Red
  digitalWrite(red_power, 255);
  analogWrite(green_power, 0);
//  digitalWrite(red_q1, 255);
//  digitalWrite(green_q1, 0);
//
//  digitalWrite(red_q2, 255);
//  digitalWrite(green_q2, 0);
//  
//  digitalWrite(red_q3, 255);
//  digitalWrite(green_q3, 0);
//  
//  digitalWrite(red_q3, 255);
//  digitalWrite(green_q3, 0);
 
  delay(2000);
 
//  switch_state = digitalRead(switch_mode);
//  
//  if(switch_state == HIGH) {
//    digitalWrite(red_power, 0);
//    analogWrite(green_power, 255);
//  }
//  else {
//    digitalWrite(red_power, 255);
//    analogWrite(green_power, 0);
//  }
 
//  digitalWrite(buzzer, LOW);
 
//  if(!once) {
//
//    once = true;
//  }
 
}

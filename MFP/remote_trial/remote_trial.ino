#include <bluefruit.h>
#include <SoftwareSerial.h>
//Adafruit_BluefruitLE_SPI ble(SPI_CS, SPI_IRQ, SPI_RST);

const int RX = 15;
const int TX = 16;

SoftwareSerial ccSerial(RX, TX);

byte receivedLeakStatus;
unsigned long debounceDelay = 50;

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
const int button_mute = 0;
byte button_mute_state = LOW;
byte button_mute_last_state = LOW;

// Set Button Pinout
const int button_set = 25;
byte button_set_state = LOW;
byte button_set_last_state = LOW;

// Up Button Pinout
const int button_up = 1;
byte button_up_state = LOW;
byte button_up_last_state = LOW;

// Down Button Pinout
const int button_down = 24;
byte button_down_state = LOW;
byte button_down_last_state = LOW;

// SLIDE SWITCH PIN DEFINITIONS -----------------------------------------

// Slide Switch Pinout
const int switch_mode = 21;
int switch_mode_state = LOW;
byte switch_mode_last_state = LOW;

// BUZZER PIN DEFINITIONS -----------------------------------------------

// Buzzer Pinout
const int buzzer = 26;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(red_power, OUTPUT);
  pinMode(green_power, OUTPUT);
 
  pinMode(red_q1, OUTPUT);
  pinMode(green_q1, OUTPUT);

  pinMode(red_q2, OUTPUT);
  pinMode(green_q2, OUTPUT);
 
  pinMode(red_q3, OUTPUT);
  pinMode(green_q3, OUTPUT);

  pinMode(red_q4, OUTPUT);
  pinMode(green_q4, OUTPUT);

  pinMode(switch_mode, INPUT);
  pinMode(button_mute, INPUT);

  pinMode(button_set, INPUT);
 
  pinMode(button_up, INPUT);
  pinMode(button_down, INPUT);
 
  pinMode(buzzer, OUTPUT);

  turnAllOff();

  setPowerColour(0, 255);

}

void setPowerColour(int red, int green){
  analogWrite(red_power, red);
  analogWrite(green_power, green);
}

void turnAllOff(){
  analogWrite(red_q1, 0);
  analogWrite(red_q2, 0);
  analogWrite(red_q3, 0);
  analogWrite(red_q4, 0);
  analogWrite(green_q1, 0);
  analogWrite(green_q2, 0);
  analogWrite(green_q3, 0);
  analogWrite(green_q4, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10000);
  setLightYellow(red_q3, green_q3);
  delay(10000);
  setLightRed(red_q3, green_q3);
  delay(10000);
  setLightOff(red_q3, green_q3);
}

void setLightYellow(int redPin, int greenPin){
  analogWrite(redPin, 255);
  analogWrite(greenPin, 50);
  Serial.println("setting light yellow");
}

void setLightRed(int redPin, int greenPin){
  analogWrite(redPin, 255);
  analogWrite(greenPin, 0);
  Serial.println("setting light red");
}

void setLightOff(int redPin, int greenPin){
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  Serial.println("setting light off");
}

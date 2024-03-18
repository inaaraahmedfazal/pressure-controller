// LED PIN DEFINITIONS --------------------------------------------------
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

// signals to be recieved from the mechanical side ----------------------
int charge_level = 3;
bool q1_small = false;
bool q1_large = false;
bool q2_small = false;
bool q2_large = false;
bool q3_small = false;
bool q3_large = false;
bool q4_small = false;
bool q4_large = false;


bool q1_large_prev = false;
bool q2_large_prev = false;
bool q3_large_prev = false;
bool q4_large_prev = false;

//signals to send/set behaviour on mechanical side ----------------------
bool inflate_all = false;
bool deflate_all = false;
bool inflate_deflate_to_preset = false;
bool record_new_value = false;

volatile bool soundAlarm = false;
volatile bool mutePressed = false;

// TIMERS  --------------------------------------------------------------
unsigned long set_pressed_time = 0; // time set new reference value button is pressed
unsigned long preset_pressed_time = 0; // time both inflate and deflate are pressed
//=======================================================================

// ACTION STATE
enum ActionState {
  MAIN_MODE = 0,
  INFLATE_ALL = 1,
  DEFLATE_ALL = 2,
  SET_NEW_REF = 3,
  INFLATE_QUAD = 4, 
  DEFLATE_QUAD = 5,
  GO_TO_REF = 6
};

void setup() {
//  ble.begin();
//  
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
  attachInterrupt(digitalPinToInterrupt(button_mute), muteButtonISR, CHANGE);

 
  pinMode(button_set, INPUT);
 
  pinMode(button_up, INPUT);
  pinMode(button_down, INPUT);
 
  pinMode(buzzer, OUTPUT);

  turnAllOff();
}


// FUNCTIONS TO SET LIGHT COLOURS
//-----------------------------------------------------------------

void setPowerColour(int red, int green){
  analogWrite(red_power, red);
  analogWrite(green_power, green);
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
void turnAllGreen(){
  analogWrite(green_q1, 255);
  analogWrite(green_q2, 255);
  analogWrite(green_q3, 225);
  analogWrite(green_q4, 255);
}

void turnAllRed(){
  analogWrite(red_q1, 225);
  analogWrite(red_q2, 255);
  analogWrite(red_q3, 225);
  analogWrite(red_q4, 255);
}

void muteButtonISR() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > 200) { // 200 ms for debounce
  if (digitalRead(button_mute) == HIGH) {
    mutePressed = true;
  }
  lastInterruptTime = interruptTime;
  }
}

void flashAllGreen(){
  turnAllOff();
  turnAllGreen();
  tone(buzzer, 440);
  delay(500);
  turnAllOff();
  noTone(buzzer);
  delay(500);
  turnAllGreen();
  tone(buzzer, 440);
  delay(500);
  turnAllOff();
  noTone(buzzer);
  delay(500);
  turnAllGreen();
  tone(buzzer, 440);
  delay(500);
  turnAllOff();
  noTone(buzzer);
}
//-----------------------------------------------------------------
 
void recordNewValue(){
  // check if lock is enabled (how do we indicate through ui if not)
  // collect 60 values
}

byte extractByteAfterSubstring(String inputString, String substring) {
  int index = inputString.indexOf(substring);
  if (index != -1 && index + substring.length() < inputString.length()) {
    char nextChar = inputString.charAt(index + substring.length());
    return static_cast<byte>(nextChar);
  }
  return 0; // Default return if substring not found or no character after it
}
void checkLeaks(){
  String serialReceived = "";
  while(ccSerial.available()) {
    serialReceived += ccSerial.read();
  }
  if (serialReceived.indexOf("LEAK STATUS->") != -1) {
    receivedLeakStatus = extractByteAfterSubstring(serialReceived, "LEAK STATUS->");
  }
  byte receivedLeakStatus = ccSerial.read(); // to modify
  q1_small = receivedLeakStatus & 1;    // Checks the 0th bit
  q1_large = receivedLeakStatus & (1 << 1); // Checks the 1st bit
  q2_small = receivedLeakStatus & (1 << 2); // Checks the 2nd bit
  q2_large = receivedLeakStatus & (1 << 3); // Checks the 3rd bit
  q3_small = receivedLeakStatus & (1 << 4); // Checks the 4th bit
  q3_large = receivedLeakStatus & (1 << 5); // Checks the 5th bit
  q4_small = receivedLeakStatus & (1 << 6); // Checks the 6th bit
  q4_large = receivedLeakStatus & (1 << 7); // Checks the 7th bit
 
  if(q1_small){
    setLightYellow(red_q1, green_q1);
  } else if (q1_large) {
    setLightRed(red_q1, green_q1);
  } else {
    setLightOff(red_q1, green_q1);
  }

  if (q2_small){
    setLightYellow(red_q2, green_q2);
  } else if (q2_large) {
    setLightRed(red_q2, green_q2);
  } else {
  setLightOff(red_q2, green_q2);
  }

  if (q3_small){
    setLightYellow(red_q3, green_q3);
  } else if (q3_large) {
    setLightRed(red_q3, green_q3);
  } else {
    setLightOff(red_q3, green_q3);
  }
 
  if (q4_small){
    setLightYellow(red_q4, green_q4);
  } else if (q4_large) {
    setLightRed(red_q4, green_q4);
  } else {
    setLightOff(red_q4, green_q4);
  }
}

void springLeak() {
  turnAllOff();
  q1_small = true;
  checkLeaks();
  Serial.println("setting q1 yellow");
  delay(5000);
  q1_small = false;
  q1_large = true;
  checkLeaks();
  Serial.println("setting q1 red");
  delay(5000);
  q1_large = false;
  q2_small = true;
  checkLeaks();
  Serial.println("setting q2 yellow");
  delay(5000);
  q2_small = false;
  q2_large = true;
  checkLeaks();
  Serial.println("setting q2 red");
  delay(5000);
  q2_large = false;
  q3_small = true;
  checkLeaks();
  Serial.println("setting q3 yellow");
  delay(5000);
  q3_small = false;
  q3_large = true;
  checkLeaks();
  Serial.println("setting q3 red");
  delay(5000);
  q3_large = false;
  q4_small= true;
  checkLeaks();
  Serial.println("setting q4 yellow");
  delay(5000);
  q4_small= false;
  q4_large = true;
  checkLeaks();
  Serial.println("setting q4 red");
  delay(5000);
  q4_large = false;
 

}

void loop() {
  // CHECK LEAK STATUS BY READING ONE BYTE FROM SERIAL MONITOR
  if ccSerial.available() {
    checkLeaks();
  }

   springLeak();
   // TO DO: will have to update to reflect charge level
   setPowerColour(0, 255);
   
   if (q1_large || q2_large || q3_large || q3_large ) {
  soundAlarm = true;
  // if there is a new large leak reset the mute to be false, triggering a new alarm
  if ((q1_large && !q1_large_prev) || (q2_large && !q2_large_prev) || (q3_large && !q3_large_prev) || (q4_large && !q4_large_prev)) {
    mutePressed = false;
  }
   } else {
  soundAlarm = false;
   }
   q1_large_prev = q1_large;
   q2_large_prev = q2_large;
   q3_large_prev = q3_large;
   q4_large_prev = q4_large;
 
   if (soundAlarm && !mutePressed ) {
  delay(300);
  tone(buzzer, 440);
  delay(300);
  turnAllOff();
  noTone(buzzer);
  Serial.println("sounding alarm");
  } else if (mutePressed){
  Serial.println("mute pressed");
  }
  switch_mode_state = digitalRead(switch_mode);
  if (switch_mode_state == LOW) { // ON
  //CHECK FOR SETTING NEW REFERENCE ---------------------------------------
    button_set_state = digitalRead(button_set);
    if (button_set_state == HIGH && button_set_last_state == LOW) {
      set_pressed_time = millis();
      Serial.println("Initial Pressed set button");
      ccSerial.write(String("ACTION STATE->" + String(ActionState.MAIN_MODE)));
      // prevent from being set to true immediately after 3 seconds are detected in the same press
      record_new_value = false;
    }
    // Check if the button is still pressed after 3 seconds
    if (button_set_state == HIGH && (millis() - set_pressed_time >= 3000) && !record_new_value) {
      record_new_value = true;
      flashAllGreen();
      Serial.println("Recording new value - LEDs should flash green");
      ccSerial.write(String("ACTION STATE->" + String(ActionState.SET_NEW_REF)));
    }
    //resets
    if (button_set_state == LOW && button_set_last_state == HIGH) {
      if (record_new_value) {
        record_new_value = false;
      }
    }
  button_set_last_state = button_set_state;
 

  //CHECK FOR DEFLATE ALL ------------------------------
  button_down_state = digitalRead(button_down);
  if (button_down_state != button_down_last_state) {
    button_down_last_state = button_down_state;
    if (button_down_state == HIGH) {
      deflate_all = true;
      Serial.println("Deflate All");
      ccSerial.write(String("ACTION STATE->" + String(ActionState.DEFLATE_ALL)));
    } else {
      deflate_all = false;
      Serial.println("Stop Deflating All");
      ccSerial.write(String("ACTION STATE->" + String(ActionState.MAIN_MODE)));
    }
  }

  //CHECK FOR INFLATE ALL ------------------------------
  button_up_state = digitalRead(button_up);
  if (button_up_state != button_up_last_state) {
    button_up_last_state = button_up_state;
    if (button_up_state == HIGH) {
      //inflate_all = true;
      Serial.println("Inflate All");
      ccSerial.write(String("ACTION STATE->" + String(ActionState.INFLATE_ALL)));
    } else {
      //inflate_all = false;
      Serial.println("Stop Inflating All");
      ccSerial.write(String("ACTION STATE->" + String(ActionState.MAIN_MODE)));
    }
  }
  }// end switch mode high (
  else{ // SWITCH IS OFF

  // CHECK FOR BOTH INFLATE DEFLATE PRESSED ----------------------------------------------------
  button_up_state = digitalRead(button_up);
  button_down_state = digitalRead(button_down);
  if (button_up_state != button_up_last_state || button_down_state != button_down_last_state) {
    button_down_last_state = button_down_state;
    button_up_last_state = button_up_state;
    if (button_up_state == HIGH && button_down_state == HIGH){
      preset_pressed_time = millis();
      Serial.println("Both pressed");
    }
  }

  if (button_up_state == HIGH && button_down_state == HIGH &&
    (millis() - preset_pressed_time  >= 3000)) {
      if (!inflate_deflate_to_preset) {
        inflate_deflate_to_preset = true;
        flashAllGreen();
        Serial.println("Both buttons held for 3 seconds (inflate/deflate to preset)");
        ccSerial.write(String("ACTION STATE->" + String(ActionState.GO_TO_REF)));
      }
    }else if (button_up_state == LOW || button_down_state == LOW) {
          inflate_deflate_to_preset = false; //reset
    }

  } // END SWITCH IS OFF
 
} // END LOOP

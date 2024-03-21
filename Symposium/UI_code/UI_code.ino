// LED PIN DEFINITIONS --------------------------------------------------
//#include <bluefruit.h>
#include <SoftwareSerial.h>
//Adafruit_BluefruitLE_SPI ble(SPI_CS, SPI_IRQ, SPI_RST);
#define TIMEOUT 2000
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
byte switch_mode_last_state = HIGH;

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


unsigned long previousMillis = 0; // Stores the last time the power function was called
const long interval = 60000; // Interval at which to call the power function (milliseconds in a minute)
//=======================================================================

// ACTION STATE
enum ActionState {
  IDLE_STATE = 0,
  UNLOCK_QUAD = 1, 
  INFLATE_ALL = 2,
  DEFLATE_ALL = 3,
  SET_NEW_REF = 4,
  INFLATE_QUAD = 5, 
  DEFLATE_QUAD = 6,
  GO_TO_REF = 7,
};

void setup() {
//  ble.begin();
//  
  Serial.begin(9600);
  ccSerial.begin(9600);
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
  analogWrite(red_power,0); // take out if causing problems
  analogWrite(green_power,0); // take out if causing problems 
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

void flashAllRed() {
  turnAllOff();
  turnAllRed();
  delay(500);

  turnAllOff();
  delay(500);

  turnAllGreen();
  delay(500);

  turnAllOff();
  delay(500);

  turnAllGreen();
  delay(500);
  turnAllOff();
  
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



void checkPowerStatus(){
  ccSerial.println("ping"); // Send a ping message
  // Wait for a reply with a timeout
  unsigned long startWait = millis();
  bool replyReceived = false;
  while (millis() - startWait < TIMEOUT) {
    if (ccSerial.available() > 0) {
      String incomingMsg = ccSerial.readString();
      if (incomingMsg.indexOf("pong") != -1) {
        replyReceived = true;
        break; 
      }
    }
  }
  // Update the power light status based on the reply
  if (replyReceived) {
      analogWrite(red_power, 0);
      analogWrite(green_power, 100); // make this lower if u want it less vibrant 
  } else {
    // set power light off
    analogWrite(red_power, 0);
    analogWrite(green_power, 0); 
  }
}

void loop() {
  // CHECK LEAK STATUS BY READING ONE BYTE FROM SERIAL MONITOR
   unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    checkPowerStatus();
  }
   
   if (q1_large || q2_large || q3_large || q3_large ) {
    soundAlarm = true;
  // if there is a new large leak reset the mute to be false, triggering a new alarm
  if ((q1_large && !q1_large_prev) || (q2_large && !q2_large_prev) || (q3_large && !q3_large_prev) || (q4_large && !q4_large_prev)) {
    mutePressed = false;
  }} else {
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
  
   if(switch_mode_state == LOW) { // ON

   //CHECK IF THEY HAVE JUST FLIPPED SWITCH -- UNLOCK QUADRANTS
    if (switch_mode_last_state == HIGH) {
      switch_mode_last_state = LOW;
      String stateMessage = "ACTION STATE->" + String(UNLOCK_QUAD) + "\n";
      ccSerial.print(stateMessage);
      Serial.println("State message" + stateMessage);
   }
   
  //CHECK FOR SETTING NEW REFERENCE ---------------------------------------
    button_set_state = digitalRead(button_set);
    if (button_set_state == HIGH && button_set_last_state == LOW) {
      set_pressed_time = millis();
      Serial.println("Initial Pressed set button");
      // prevent from being set to true immediately after 3 seconds are detected in the same press
      record_new_value = false;
    }
    // Check if the button is still pressed after 3 seconds
    if (button_set_state == HIGH && (millis() - set_pressed_time >= 3000) && !record_new_value) {
      record_new_value = true;
      flashAllGreen();
      Serial.println("Recording new value - LEDs should flash green");
      String stateMessage = "ACTION STATE->" + String(SET_NEW_REF) + "\n";
      ccSerial.print(stateMessage);
      Serial.println("State message" + stateMessage);
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
      String stateMessage = "ACTION STATE->" + String(DEFLATE_ALL) + "\n";
      ccSerial.print(stateMessage);
    } else {
      deflate_all = false;
      Serial.println("Stop Deflating All");
      String stateMessage = "ACTION STATE->" + String(UNLOCK_QUAD) + "\n";
      ccSerial.print(stateMessage);
    }
  }

  //CHECK FOR INFLATE ALL ------------------------------
  button_up_state = digitalRead(button_up);
  if (button_up_state != button_up_last_state) {
    button_up_last_state = button_up_state;
    if (button_up_state == HIGH) {
      //inflate_all = true;
      Serial.println("Inflate All");
      String stateMessage = "ACTION STATE->" + String(INFLATE_ALL) + "\n";
      Serial.println(stateMessage);
      ccSerial.print(stateMessage);
    } else {
      //inflate_all = false;
      Serial.println("Stop Inflating All");
      String stateMessage = "ACTION STATE->" + String(UNLOCK_QUAD) + "\n";
      Serial.println(stateMessage);
      ccSerial.print(stateMessage);
    }
  }
  }// end switch mode high (
  else{ // SWITCH IS OFF
    
   //CHECK IF THEY HAVE JUST FLIPPED SWITCH TO OFF -- LOCK QUADRANTS
    if (switch_mode_last_state == LOW) {
      switch_mode_last_state = HIGH;
      String stateMessage = "ACTION STATE->" + String(IDLE_STATE) + "\n";
      ccSerial.print(stateMessage);
    }

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
        String stateMessage = "ACTION STATE->" + String(GO_TO_REF) + "\n";
        ccSerial.print(stateMessage);
      }
    }else if (button_up_state == LOW || button_down_state == LOW) {
       inflate_deflate_to_preset = false; //reset
    }

  } // END SWITCH IS OFF
 
} // END LOOP

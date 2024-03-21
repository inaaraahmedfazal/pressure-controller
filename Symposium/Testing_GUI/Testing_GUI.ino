// declaring output pins
int valve_1 = 2;
int valve_2 = 3;
int valve_3 = 4;
int valve_4 = 5;
int exhaust = 6;
int pump    = 7;

boolean valve_1_state = false;
boolean valve_2_state = false;
boolean valve_3_state = false;
boolean valve_4_state = false;
boolean exhaust_state = false;
boolean pump_state    = false;

// declaring input pins
//int v_sensor = A0;
//int sensor_1 = A1;
//int sensor_2 = A2;
//int sensor_3 = A3;
//int sensor_4 = A4;

unsigned long previousMillis = 0; // Variable to store the last time analog values were printed
const long interval = 500;

void setup() {

  pinMode(valve_1, OUTPUT);
  pinMode(valve_2, OUTPUT);
  pinMode(valve_3, OUTPUT);
  pinMode(valve_4, OUTPUT);
  pinMode(exhaust, OUTPUT);
  pinMode(pump,    OUTPUT);
  
  Serial.begin(9600);

}

void loop() {

  unsigned long currentMillis = millis(); // Get the current time

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // Update the last print time
   
    for (int i = 0; i < 4; i++) {
      float sensorValue = analogRead(A1 + i);
  
      if (i == 0) {
        sensorValue = 0.00792*sensorValue + -0.252;
      }
      if (i == 1) {
        sensorValue = 0.00789*sensorValue + -0.302;
      }
      if (i == 2) {
        sensorValue = 0.00792*sensorValue + -0.281;
      }
      if (i == 3) {
        sensorValue = 0.00789*sensorValue + -0.262;
      }

      if (sensorValue < 0){
        sensorValue = 0;
      }
      Serial.print(sensorValue);
      Serial.print(",");
    }
    Serial.println(); // End of data line
  }
    
  if (Serial.available()) {

    char val = Serial.read();

    if (val == 'a') {    
      if (valve_1_state == false) {
        valve_1_state = true;
        digitalWrite(valve_1, HIGH); 
      } else {
        valve_1_state = false;
        digitalWrite(valve_1, LOW ); 
      }
    }

    if (val == 'b') {    
      if (valve_2_state == false) {
        valve_2_state = true;
        digitalWrite(valve_2, HIGH); 
      } else {
        valve_2_state = false;
        digitalWrite(valve_2, LOW ); 
      }
    }

    if (val == 'c') {    
      if (valve_3_state == false) {
        valve_3_state = true;
        digitalWrite(valve_3, HIGH); 
      } else {
        valve_3_state = false;
        digitalWrite(valve_3, LOW ); 
      }
    }

    if (val == 'd') {    
      if (valve_4_state == false) {
        valve_4_state = true;
        digitalWrite(valve_4, HIGH); 
      } else {
        valve_4_state = false;
        digitalWrite(valve_4, LOW ); 
      }
    }

    if (val == 'e') {    
      if (exhaust_state == false) {
        exhaust_state = true;
        digitalWrite(exhaust, HIGH); 
      } else {
        exhaust_state = false;
        digitalWrite(exhaust, LOW ); 
      }
    }

    if (val == 'f') {    
      if (pump_state == false) {
        pump_state = true;
        digitalWrite(pump, HIGH); 
      } else {
        pump_state = false;
        digitalWrite(pump, LOW ); 
      }
    }
  
  }
}

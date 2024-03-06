//SOLENOID VALVES
const int VALVE1 = 9;
const int VALVE2 = 10;
const int VALVE3 = 11;
const int VALVE4 = 12;
const int VALVE_OUT = 13;
const int PUMP = 8;

void setup() {
  // put your setup code here, to run once:
  pinMode(VALVE1, OUTPUT); // Set the VALVE pin as an output
  pinMode(VALVE2, OUTPUT); // Set the VALVE pin as an output
  pinMode(VALVE3, OUTPUT); // Set the VALVE pin as an output
  pinMode(VALVE4, OUTPUT); // Set the VALVE pin as an output
  pinMode(VALVE_OUT, OUTPUT); // Set the VALVE pin as an output
  pinMode(PUMP, OUTPUT);
  Serial.begin(9600);
}

void reset() {
  digitalWrite(VALVE1, LOW);
  digitalWrite(VALVE2, LOW);
  digitalWrite(VALVE3, LOW);
  digitalWrite(VALVE4, LOW);
  digitalWrite(VALVE_OUT, LOW);

}

void inflateQuadrant1() {
  reset();
  digitalWrite(PUMP, HIGH);
  digitalWrite(VALVE1, HIGH);
  digitalWrite(PUMP, HIGH);
  Serial.println("Opening Valve 1 with pump on");
  delay(15000);
}

void inflateQuadrant2() {
  reset();
  digitalWrite(PUMP, HIGH);
  digitalWrite(VALVE2, HIGH);
  Serial.println("Opening Valve 2 with pump on");
  delay(15000);
}

void inflateQuadrant3() {
  reset();
  digitalWrite(PUMP, HIGH);
  digitalWrite(VALVE3, HIGH);
  Serial.println("Opening Valve 3 with pump on");
  delay(15000);
}

void inflateQuadrant4(){
  reset();
  digitalWrite(PUMP, HIGH);
  digitalWrite(VALVE4, HIGH);
  Serial.println("Opening Valve 4 with pump on");
  delay(15000);
}

void deflateQuadrant4(){
  reset();
  digitalWrite(PUMP, LOW);
  digitalWrite(VALVE4, HIGH);
  digitalWrite(VALVE_OUT, HIGH);
  Serial.println("Opening Valve 4 and Exhaust");
  delay(15000);
}

void deflateAll(){
  digitalWrite(PUMP, LOW);
  digitalWrite(VALVE1, HIGH);
  digitalWrite(VALVE2, HIGH);
  digitalWrite(VALVE3, HIGH);
  digitalWrite(VALVE4, HIGH);
  digitalWrite(VALVE_OUT, HIGH);
  Serial.println("Opening all except pump (Deflate all)");
  delay(15000);
}

void inflateAll(){
  digitalWrite(PUMP, HIGH);
  digitalWrite(VALVE1, HIGH);
  digitalWrite(VALVE2, HIGH);
  digitalWrite(VALVE3, HIGH);
  digitalWrite(VALVE4, HIGH);
  digitalWrite(VALVE_OUT, LOW);
  Serial.println("INflate all");
  delay(15000);
}



void loop() {
  // put your main code here, to run repeatedly:
  //Exhaust open, others closed
  // digitalWrite(VALVE_OUT, LOW);
  // digitalWrite(VALVE1, LOW);
  // digitalWrite(VALVE2, LOW);
  // digitalWrite(VALVE3, LOW);
  // digitalWrite(VALVE4, HIGH);
  // digitalWrite(PUMP, HIGH);
  // Serial.println("Opening Valve 1 with pump on");
  // delay(15000);

  inflateQuadrant2();

  // digitalWrite(PUMP, HIGH);
  // digitalWrite(VALVE4, LOW);
  // Serial.println("Turning pump off");


}

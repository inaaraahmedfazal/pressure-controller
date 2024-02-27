//SOLENOID VALVES
const int RELAY1 = 4;
const int RELAY2 = 5;
const int RELAY3 = 6;
const int RELAY4 = 7;
const int RELAY_OUT = 8;
const int RELAY_PUMP = 3;

void setup() {
  // put your setup code here, to run once:
  pinMode(RELAY1, OUTPUT); // Set the relay pin as an output
  pinMode(RELAY2, OUTPUT); // Set the relay pin as an output
  pinMode(RELAY3, OUTPUT); // Set the relay pin as an output
  pinMode(RELAY4, OUTPUT); // Set the relay pin as an output
  pinMode(RELAY_OUT, OUTPUT); // Set the relay pin as an output
  pinMode(RELAY_PUMP, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  //Exhaust open, others closed
  digitalWrite(RELAY_OUT, HIGH);
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  digitalWrite(RELAY3, HIGH);
  digitalWrite(RELAY4, LOW);
  digitalWrite(RELAY_PUMP, HIGH);
  delay(5000);

  digitalWrite(RELAY2, LOW);
  digitalWrite(RELAY4, HIGH);
  delay(5000);

  digitalWrite(RELAY3, LOW);
  digitalWrite(RELAY2, HIGH);
  delay(5000);

  digitalWrite(RELAY4, LOW);
  digitalWrite(RELAY3, HIGH);
  delay(5000);

  digitalWrite(RELAY_OUT, LOW);
  digitalWrite(RELAY4, HIGH);
  delay(5000);

  digitalWrite(RELAY_PUMP, LOW);
  digitalWrite(RELAY_OUT, HIGH);
  delay(5000);
}

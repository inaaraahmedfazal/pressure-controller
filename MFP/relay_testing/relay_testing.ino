//SOLENOID VALVES
const int RELAY1 = 4;
const int RELAY2 = 5;
const int RELAY3 = 6;
const int RELAY4 = 7;
const int RELAY5 = 8;

void setup() {
  Serial.begin(9600);  // Initialize serial communication
  pinMode(RELAY1, OUTPUT); // Set the relay pin as an output
  pinMode(RELAY2, OUTPUT); // Set the relay pin as an output
  pinMode(RELAY3, OUTPUT); // Set the relay pin as an output
  pinMode(RELAY4, OUTPUT); // Set the relay pin as an output
  pinMode(RELAY5, OUTPUT); // Set the relay pin as an output
}
bool once = false;
void loop() {
  delay(5000);
  Serial.println("Starting Relay Test");

  delay(1000);
  Serial.println("Setting Relay 1 LOW:");
  delay(1000);
  digitalWrite(RELAY1, LOW);
  delay(1000);
  Serial.println("Setting Relay 1 HIGH:");
  delay(1000);
  digitalWrite(RELAY1, HIGH);

  delay(1000);
  Serial.println("Setting Relay 2 LOW:");
  delay(1000);
  digitalWrite(RELAY2, LOW);
  delay(1000);
  Serial.println("Setting Relay 2 HIGH:");
  delay(1000);
  digitalWrite(RELAY2, HIGH);

  delay(1000);
  Serial.println("Setting Relay 3 LOW:");
  delay(1000);
  digitalWrite(RELAY3, LOW);
  delay(1000);
  Serial.println("Setting Relay 3 HIGH:");
  delay(1000);
  digitalWrite(RELAY3, HIGH);

  delay(1000);
  Serial.println("Setting Relay 4 LOW:");
  delay(1000);
  digitalWrite(RELAY4, LOW);
  delay(1000);
  Serial.println("Setting Relay 4 HIGH:");
  delay(1000);
  digitalWrite(RELAY4, HIGH);

  delay(1000);
  Serial.println("Setting Relay 5 LOW:");
  delay(1000);
  digitalWrite(RELAY5, LOW);
  delay(1000);
  Serial.println("Setting Relay 5 HIGH:");
  delay(1000);
  digitalWrite(RELAY5, HIGH);
  
  once = true;
  
  delay(5000);
}
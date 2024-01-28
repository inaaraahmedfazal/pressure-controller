//PRESSURE TRANSDUCER
const int pressureSensorPin = A0;  // Define the analog input pin for the pressure transducer //PIN?

//SOLENOID VALVES
const int valveRelayPinIN = 12; // The digital pin connected to the relay module control pin //PIN?
const int valveRelayPinOUT = 11; // The digital pin connected to the relay module control pin //PIN?

void setup() {
  Serial.begin(9600);  // Initialize serial communication
  pinMode(valveRelayPinIN, OUTPUT); // Set the relay pin as an output
  pinMode(valveRelayPinOUT, OUTPUT); // Set the relay pin as an output
}
bool once = false;
void loop() {
  int sensorPressure = analogRead(pressureSensorPin);
  float pressureInPSI = 7.93*pow(10, -3)*sensorPressure - 0.35;
  Serial.print("Pressure Analog: ");
  Serial.println(sensorPressure);
//
  Serial.print("Pressure PSI: ");
  Serial.println(pressureInPSI);
  Serial.println("HERE");
  digitalWrite(12, LOW);
  digitalWrite(11, HIGH);

  delay(5000);

  digitalWrite(12, HIGH);
  digitalWrite(11, HIGH);
  once = true;
  
  delay(5000);
}

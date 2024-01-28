//PRESSURE TRANSDUCER
const int pressureSensorPin = A0;  // Define the analog input pin for the pressure transducer 
const float VCC = 5.0;             // Supply voltage for the transducer (adjust as needed)
const float sensorRange = 1023.0;  // Range of the analog-to-digital converter (0-1023)
// Datasheet - sensor's characteristics
const float V_offset = 4.7;       // Offset voltage from datasheet
const float Sensitivity = (90.0*0.1450377377)/1000; //Sensitivity in PSI per volt from datasheet (mV/KPa -> PSI)

//SOLENOID VALVES
const int valveRelayPinIN = 12; // The digital pin connected to the relay module control pin 
const int valveRelayPinOUT = 11; // The digital pin connected to the relay module control pin 

//PUMP
//const int pumpPin = 6; //The digital pin connected to the 5V pump

//TARGET PRESSURE RANGE
const float maxPressure = 0;
const float minPressure = 0;

void setup() {
  Serial.begin(9600);  // Initialize serial communication
  pinMode(valveRelayPinIN, OUTPUT); // Set the relay pin as an output
  pinMode(valveRelayPinOUT, OUTPUT); // Set the relay pin as an output
  pinMode(pumpPin, OUTPUT); //Set the pump pin as an output
}

void loop() {
  // Read the analog voltage from the pressure transducer
  int sensorPressure = analogRead(pressureSensorPin);

  // Convert analog reading to voltage
  float voltageReading = sensorPressure * (VCC / sensorRange);
  //Convert voltage to PSI
  float pressureInPSI = (voltageReading - V_offset) / Sensitivity;

  // Display the pressure reading in PSI
  Serial.print("Pressure (PSI): ");
  Serial.println(pressureInPSI);

  delay(1000);  // Delay for one second before taking another reading
  
  if(pressureInPSI > maxPressure){ //Let air out
    ValveInOn();
    ValveOutOn();
  }
  else if (pressureInPSI < minPressure) //Let air in
  {
    ValveInOn();
    PumpOn();
  }
  else { //Maintain air
    ValveInOff();
    ValveOutOff();
    PumpOff();
  }
}

//Pump ON/OFF logic
void PumpOn(){
  digitalWrite(pumpPin, HIGH); // Activate the relay
}

void PumpOff(){
  digitalWrite(pumpPin, LOW); // Activate the relay
}

// Valve ON/OFF logic
void ValveInOn() {
  digitalWrite(valveRelayPinIN, HIGH); // Activate the relay
}

void ValveInOff() {
  digitalWrite(valveRelayPinIN, LOW); // Deactivate the relay
}

void ValveOutOn() {
  digitalWrite(valveRelayPinOUT, HIGH); // Activate the relay
}

void ValveOutOff() {
  digitalWrite(valveRelayPinOUT, LOW); // Deactivate the relay
}

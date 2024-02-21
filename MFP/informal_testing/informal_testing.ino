#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define N 15
#define M 4

const int sensorPin1 = A0;
const int sensorPin2 = A1;
const int sensorPin3 = A2;
const int sensorPin4 = A3;

static unsigned long lastRefreshTime = 0;
const int sampling_interval = 1000;
static unsigned int sampleCount = 0;

class Sensor {
  float calibration_m;
  float calibration_b;
  int quadrant;
  int pin;
  public:
  Sensor::Sensor(int q, int p, float m, float b) {
    quadrant = q;
    pin = p;
    calibration_m = m;
    calibration_b = b;
  }

  void Sensor::readPSI() {
    // Read the analog voltage from the pressure transducer
    int sensorPressure = analogRead(this->pin);
    float pressureInPSI = this->calibration_m*sensorPressure + this->calibration_b;

  //  // Convert analog reading to voltage
  //  float voltageReading = sensorPressure * (VCC / sensorRange);
  //  //Convert voltage to PSI
  //  //float pressureInPSI = (voltageReading - V_offset) / Sensitivity;
  //  float pressureInPSI = voltageReading / Sensitivity;
  //  // Display the pressure reading in PSI
  //
    Serial.print("Pressure PSI for Quadrant ");
    Serial.print(this->quadrant);
    Serial.print(": ");
    Serial.println(pressureInPSI);
  }
};

Sensor s1(1, sensorPin1, 0.00792, -0.252);
Sensor s2(2, sensorPin2, 0.00789, -0.302);
Sensor s3(3, sensorPin3, 0.00792, -0.281);
Sensor s4(4, sensorPin4, 0.00789, -0.262);

//TESTING CALIBRATION SWAP
//Sensor s3(3, sensorPin3, 0.00789, -0.262);
//Sensor s4(4, sensorPin4, 0.00792, -0.281);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //Initialize serial communication

}



void loop() {
  // put your main code here, to run repeatedly:
  if(sampleCount < 1200) {
    if(millis() - lastRefreshTime >= sampling_interval) {
      Serial.println("Starting sample:");
      lastRefreshTime = millis();
      s1.readPSI();
      s2.readPSI();
      s3.readPSI();
      s4.readPSI();
      sampleCount++;
      if(sampleCount == 1200) {
        Serial.println("end");
      }
    }
  }

}

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define N 15
#define M 4



//PRESSURE TRANSDUCER
const int pressureSensorPin = A0;  // Define the analog input pin for the pressure transducer 
const float VCC = 5.0;             // Supply voltage for the transducer (adjust as needed)
const float sensorRange = 1023.0;  // Range of the analog-to-digital converter (0-1023)
// Datasheet - sensor's characteristics
const float V_offset = 4.7;       // Offset voltage from datasheet
//const float Sensitivity = (90.0*0.1450377377)/1000; //Sensitivity in PSI per volt from datasheet (mV/KPa -> PSI)

//SOLENOID VALVES
const int valveRelayPinIN = 12; // The digital pin connected to the relay module control pin 
const int valveRelayPinOUT = 11; // The digital pin connected to the relay module control pin

//User Interface - LEDs and buttons
const int redLED = 2;
const int yellowLED = 3;

static float avgSet = 0.0;
static int avgSetCount = 0;

bool powerOn;
bool locked;

const int powerButton = 7;
const int lockButton = 6;
const int setButton = 5;

int powerButtonState = 0; 
int lockButtonState = 0; 
int setButtonState = 0; 

int powerButtonPreviousState = LOW;
int lockButtonPreviousState = LOW; 
int setButtonPreviousState = LOW; 

//PUMP
//const int pumpPin = 6; //The digital pin connected to the 5V pump

//TARGET PRESSURE RANGE
const float maxPressure = 90;
const float minPressure = 40;

const float acceptable_threshold = 0.1;
const float residual_error_threshold = 0.1;
const float STD_DEV_MULTIPLIER = 1.5;

float ideal_pressure = 0.6; //changeable by OT!

const float errorThreshold = 0.1;
const float bottomOut = 0.4;

const int sampling_interval = 1000;
const int control_interval = 1000;

//BOOLEAN FLAGS
bool isRefilling = false;
int refillCounter = 0;

struct Queue {
    int front, rear;
    int maxSize;
    float* items;
};

class Sensor {
  float calibration_m;
  float calibration_b;
  int quadrant;
  int pin;
  struct Queue sensorData;
  struct Queue actuationsHistory;

  Sensor(q, p, m, b) {
    quadrant = q;
    calibration_m = m;
    calibration_b = b;
    initializeQueue(&sensorData, N);
    initializeQueue(&actuationsHistory, M);
  }

  float readPSI(){
    // Read the analog voltage from the pressure transducer
    int sensorPressure = analogRead(pressureSensorPin);
    float pressureInPSI = this.calibration_m*sensorPressure + this.calibration_b;

  //  // Convert analog reading to voltage
  //  float voltageReading = sensorPressure * (VCC / sensorRange);
  //  //Convert voltage to PSI
  //  //float pressureInPSI = (voltageReading - V_offset) / Sensitivity;
  //  float pressureInPSI = voltageReading / Sensitivity;
  //  // Display the pressure reading in PSI
    Serial.print("Pressure Analog for Quadrant ");
    Serial.print(this.quadrant);
    Serial.print(": ")
    Serial.println(sensorPressure);
  //
    Serial.print("Pressure PSI for Quadrant ");
    Serial.print(this.quadrant);
    Serial.print(": ")
    Serial.println(pressureInPSI);

    return pressureInPSI;
  }
};

// Function to initialize a queue with a specified maximum size
void initializeQueue(struct Queue* queue, int size) {
    queue->front = -1;
    queue->rear = -1;
    queue->maxSize = size;
    queue->items = (float*)malloc(size * sizeof(float));
}

// Function to check if the queue is empty
int isEmpty(struct Queue* queue) {
    return (queue->front == -1 && queue->rear == -1);
}

// Function to check if the queue is full
int isFull(struct Queue* queue) {
    return (queue->rear + 1) % queue->maxSize == queue->front;
}

// Function to add an element to the rear of the queue
void enqueue(struct Queue* queue, float value) {
    if (isFull(queue)) {
        printf("Queue is full. Cannot enqueue %d\n", value);
        return;
    } else if (isEmpty(queue)) {
        queue->front = queue->rear = 0;
    } else {
        queue->rear = (queue->rear + 1) % queue->maxSize;
    }

    queue->items[queue->rear] = value;
}


// Function to remove an element from the front of the queue
float dequeue(struct Queue* queue) {
    float dequeuedItem;

    if (isEmpty(queue)) {
        printf("Queue is empty. Cannot dequeue\n");
        return -1; // Return a special value to indicate an error
    } else if (queue->front == queue->rear) {
        dequeuedItem = queue->items[queue->front];
        queue->front = queue->rear = -1;
    } else {
        dequeuedItem = queue->items[queue->front];
        queue->front = (queue->front + 1) % queue->maxSize;
    }

    return dequeuedItem;
}

// Function to get the front element of the queue without removing it
float front(struct Queue* queue) {
    if (isEmpty(queue)) {
        printf("Queue is empty\n");
        return -1; // Return a special value to indicate an error
    }

    return queue->items[queue->front];
}

// Function to calculate the mean of all values in the queue
float calculateMean(struct Queue* queue) {
    if (isEmpty(queue)) {
        printf("Queue is empty. Cannot calculate mean.\n");
        return -1.0; // Return a special value to indicate an error
    }

    float sum = 0.0;
    int count = 0;
    int i = queue->front;

    do {
        sum += queue->items[i];
        count++;
        i = (i + 1) % queue->maxSize;
    } while (i != (queue->rear + 1) % queue->maxSize);

    return sum / count;
}

// Function to calculate the standard deviation of all values in the queue
float calculateStdDev(struct Queue* queue) {
  if (isEmpty(queue)) {
    printf("Queue is empty. Cannot calculate standard deviation.\n");
    return -1.0; // Return a special value to indicate an error
  }

  float mean = calculateMean(queue);
  float sumSquaredDiff = 0.0;
  int count = 0;
  int i = queue->front;

  do {
    float diff = queue->items[i] - mean;
    sumSquaredDiff += diff * diff;
    count++;
    i = (i + 1) % queue->maxSize;
  } while (i != (queue->rear + 1) % queue->maxSize);

  return sqrt(sumSquaredDiff / count);
}

// Function to calculate the mean of the queue
float getMovingAverage(struct Queue* queue){
  float mean = calculateMean(queue);
  float std = calculateStdDev(queue);
  if (isEmpty(queue)) {
      printf("Queue is empty. Cannot calculate mean.\n");
      return -1.0; // Return a special value to indicate an error
  }
  
  float sum = 0.0;
  int count = 0;
  int i = queue->front;
  
  do {
    if(abs(queue->items[i] - mean) <= STD_DEV_MULTIPLIER * std){
      sum += queue->items[i];
      count++;
    }
    i = (i + 1) % queue->maxSize;
  } while (i != (queue->rear + 1) % queue->maxSize);
  
  return sum / count;
}

// Function to free the memory allocated for the queue
void destroyQueue(struct Queue* queue) {
    free(queue->items);
    queue->front = queue->rear = -1;
}

struct Queue sensorData;
struct Queue actuationsHistory;

void setup() {
  Serial.begin(9600);  // Initialize serial communication
  pinMode(valveRelayPinIN, OUTPUT); // Set the relay pin as an output
  pinMode(valveRelayPinOUT, OUTPUT); // Set the relay pin as an output

  //Set LEDs as outputs
  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  
  //Set buttons as inputs
  pinMode(powerButton, INPUT);
  pinMode(lockButton, INPUT);
  pinMode(setButton, INPUT);
  powerOn = false;
  locked = true;
    
  //  pinMode(pumpPin, OUTPUT); //Set the pump pin as an 
  initializeQueue(&sensorData, N);
  initializeQueue(&actuationsHistory, M);
  ValveInOff();
  ValveOutOn();

  s1 = Sensor(1, 0.00792, -0.252);
  s2 = Sensor(2, 0.00789, -0.302);
  s3 = Sensor(3, 0.00792, -0.281);
  s4 = Sensor(4, 0.00789, -0.262);
}
static unsigned long lastRefreshTime = 0;
void loop() {
  powerButtonState = digitalRead(powerButton);
  lockButtonState = digitalRead(lockButton);
  setButtonState = digitalRead(setButton);
  
  if (powerButtonState == LOW && powerButtonPreviousState == HIGH) {
    powerOn = !powerOn;
    Serial.println("powerOn: ");
    Serial.print(powerOn);
    Serial.println();
  }
   if (powerOn && lockButtonState == LOW && lockButtonPreviousState == HIGH) {
    locked = !locked;
    Serial.print("locked: ");
    Serial.print(locked);
    Serial.println();
  }
  if (powerOn && setButtonState == LOW && setButtonPreviousState == HIGH) {
    ideal_pressure = avgSet / avgSetCount;
    avgSet = 0.0;
    avgSetCount = 0;
  }
  if (powerOn && setButtonState == HIGH ) {
    // read in input 
    // set new value
    if (!locked) {
       Serial.print("setButtonPressed ");
       avgSet += sensorReadingPSI();
       avgSetCount += 1;
       Serial.println();
    } else if (setButtonPreviousState == LOW) {
      Serial.print("Must unlock");
      Serial.println();
    }
   
  } 

  setButtonPreviousState = setButtonState;
  powerButtonPreviousState = powerButtonState;
  lockButtonPreviousState = lockButtonState;
  if(powerOn && locked){
    if(millis() - lastRefreshTime >= sampling_interval) {
      lastRefreshTime = millis();
      float pressureInPSI = sensorReadingPSI();
      if(isRefilling) {
        refillMode(pressureInPSI);
      }
      else{
        mainMode(pressureInPSI);
      }
    }
  }
}

float sensorReadingPSI(){
  // Read the analog voltage from the pressure transducer
  int sensorPressure = analogRead(pressureSensorPin);
  float pressureInPSI = 7.93*pow(10, -3)*sensorPressure - 0.35;

//  // Convert analog reading to voltage
//  float voltageReading = sensorPressure * (VCC / sensorRange);
//  //Convert voltage to PSI
//  //float pressureInPSI = (voltageReading - V_offset) / Sensitivity;
//  float pressureInPSI = voltageReading / Sensitivity;
//  // Display the pressure reading in PSI
  Serial.print("Pressure Analog: ");
  Serial.println(sensorPressure);
//
  Serial.print("Pressure PSI: ");
  Serial.println(pressureInPSI);

  return pressureInPSI;
}

void refillMode(float pressureInPSI){
  if(pressureInPSI >= ideal_pressure) {
    isRefilling = false;
    ValveInOff();
    ValveOutOff();
    Serial.println("---REFILL COMPLETE---");
    for(int i = 0; i < N; i++){
      dequeue(&sensorData);
    }
  }
}

void mainMode(float pressureInPSI){
  enqueue(&sensorData, pressureInPSI);
  if(isFull(&sensorData)) {
    if(is_pressure_unacceptable(pressureInPSI){
      float moving_average = getMovingAverage(&sensorData);
      Serial.print("Moving Average: ");
      Serial.println(moving_average);
      if(isAverageUnacceptable(moving_average && pressureInPSI > -10){
        Serial.println("---REFILLING---");
        ValveInOn();
        ValveOutOff();
        isRefilling = true;
        if(isFull(&actuationsHistory)) {
          dequeue(&actuationsHistory);
        }
        Serial.println("ENQUEUEING ACTUATION 1.0");
        enqueue(&actuationsHistory, 1.0);
      }
      
    }
    else {
      if(isFull(&actuationsHistory)) {
        dequeue(&actuationsHistory);
      }
      Serial.println("ENQUEUEING ACTUATION 0.0");
      enqueue(&actuationsHistory, 0.0);
    }
    if(isFull(&actuationsHistory) && calculateMean(&actuationsHistory) == 1.0){
      Serial.println("RED LIGHT ON!");
      digitalWrite(redLED, HIGH);
    }
    else {
      digitalWrite(redLED, LOW);
    }
  }
}

// Valve ON/OFF logic
void ValveInOn() {
  digitalWrite(valveRelayPinIN, LOW); // Activate the relay
}

void ValveInOff() {
  digitalWrite(valveRelayPinIN, HIGH); // Deactivate the relay
}

void ValveOutOn() {
  digitalWrite(valveRelayPinOUT, LOW); // Activate the relay
}

void ValveOutOff() {
  digitalWrite(valveRelayPinOUT, HIGH); // Deactivate the relay
}

// Algorithm helper functions
bool is_pressure_unacceptable(float current_pressure){
  return abs(current_pressure - ideal_pressure) > acceptable_threshold;
}
bool isAverageUnacceptable(float moving_average){
  return abs(moving_average - ideal_pressure) > acceptable_threshold && moving_average > -10;
}

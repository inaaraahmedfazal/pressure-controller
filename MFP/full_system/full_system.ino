#define N 5
#define M 4

//SOLENOID VALVES
const int VALVE1 = 9;
const int VALVE2 = 10;
const int VALVE3 = 11;
const int VALVE4 = 12;
const int VALVE_OUT = 13;
const int PUMP = 8;

const int sensorPin1 = A0;
const int sensorPin2 = A1;
const int sensorPin3 = A2;
const int sensorPin4 = A3;

static unsigned long lastRefreshTime = 0;
const int sampling_interval = 1000;
static unsigned int min_ctr = 0;
const float acceptable_threshold = 0.1;
static unsigned int sampleCount = 0;


const float STD_DEV_MULTIPLIER = 1.5;

bool pump_on = false;
bool exhaust_on = false;

struct Queue {
  int front, rear;
  int maxSize;
  float* items;
};

// Function to initialize a queue with a specified maximum size
void initializeQueue(struct Queue* queue, int size) {
    queue->front = -1;
    queue->rear = -1;
    queue->maxSize = size;
    queue->items = (float*)malloc(size * sizeof(float));
}

void safeSerialPrint(String msg) {
  while (Serial.availableForWrite() == 0) {
    delay(1); // Wait for the buffer to become available
  }
  Serial.print(msg);
}

struct LeakLog {
  unsigned long detectionTimestamps[4]; // Store up to 4 detections per hour
  int count; // Current number of detections
};

//void checkSignificantLeak(LeakLog &log, bool &hasSignificantLeak) {
//  unsigned long currentTime = millis();
//  int recentDetections = 0;
//
//  for (int i = 0; i < log.count; i++) {
//    if (currentTime - log.detectionTimestamps[i] <= 3600000) { // Check the last hour
//      recentDetections++;
//    }
//  }
//
//  // Significant leak if more than two detections in the last hour
//  hasSignificantLeak = recentDetections > 2;
//}



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
class Quadrant {
  float calibration_m;
  float calibration_b;
  int num;
  int sensor_pin;
  int valve_pin;
  LeakLog leak_log;

  public:
    bool is_refilling;
    bool is_deflating;
    float ideal_pressure;
    float minuteData[60];
    struct Queue* sensorData;
    struct Queue* actuationsHistory;
  Quadrant(int n, int sp, int vp, float m, float b, float ip) {
    num = n;
    sensor_pin = sp;
    valve_pin = vp;
    calibration_m = m;
    calibration_b = b;
    is_refilling = false;
    is_deflating = false;
    ideal_pressure = ip;
    initializeQueue(sensorData, N);
    initializeQueue(actuationsHistory, M);
    for(int i = 0; i < 4; i++) {
      leak_log.detectionTimestamps[i] = 0;
    }
     leak_log.count = 0; 
  }
  
  void openValve(){
    digitalWrite(valve_pin, HIGH);
  }
  void closeValve(){
    digitalWrite(valve_pin, LOW);
  }
  float readPSI(){
    // Read the analog voltage from the pressure transducer
    int sensorPressure = analogRead(this->sensor_pin);
    float pressureInPSI = this->calibration_m*sensorPressure + this->calibration_b;
    safeSerialPrint("Pressure PSI for Quadrant " + String(this->num) + ": " + String(pressureInPSI) + "\n");
    return pressureInPSI;
  }

  float minAvg(){
    float sum = 0.0;
    // Calculate the sum of all elements in the array
    for (int i = 0; i < 60; i++) {
      sum += minuteData[i];
    }

    // Calculate the average
    return sum / 60.0;

  }

  void updateLeakLog() {
    unsigned long currentTime = millis();
    if (leak_log.count < 4) {
      leak_log.detectionTimestamps[leak_log.count++] = currentTime;
    } else {
      // Shift older timestamps to make room for the new one
      for (int i = 1; i < 4; i++) {
        leak_log.detectionTimestamps[i - 1] = leak_log.detectionTimestamps[i];
      }
      // Insert the new timestamp at the last position
      leak_log.detectionTimestamps[3] = currentTime;
    }
  }
  
};

//CALIBRATION - set the initial values 
Quadrant q1(1, sensorPin1, VALVE1, 0.00792, -0.252, 0.54);
Quadrant q2(2, sensorPin2, VALVE2, 0.00789, -0.302, 0.59);
Quadrant q3(3, sensorPin3, VALVE3, 0.00792, -0.281, 0.70);
Quadrant q4(4, sensorPin4, VALVE4, 0.00789, -0.262, 0.20);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  // Initialize serial communication
  pinMode(VALVE1, OUTPUT); 
  pinMode(VALVE2, OUTPUT); 
  pinMode(VALVE3, OUTPUT); 
  pinMode(VALVE4, OUTPUT); 
  pinMode(VALVE_OUT, OUTPUT); 
  pinMode(PUMP, OUTPUT); 
}

void loop() {
  // put your main code here, to run repeatedly:
  if(sampleCount < 1200) {
    if(millis() - lastRefreshTime >= sampling_interval) {
      safeSerialPrint("Starting sample:\n");
      lastRefreshTime = millis();

      if(q1.is_refilling || q2.is_refilling || q3.is_refilling || q4.is_refilling) {
        refillMode();
      }
      else {
        if(pump_on) {
          digitalWrite(PUMP, LOW);
          pump_on = false;
        }
        if (q1.is_deflating || q2.is_deflating|| q3.is_deflating || q4.is_deflating) {
          deflateMode();
        } else if (exhaust_on){
          digitalWrite(VALVE_OUT, LOW);
          exhaust_on = false;
        } else {
          q1.minuteData[min_ctr] = q1.readPSI();
          q2.minuteData[min_ctr] = q2.readPSI();
          q3.minuteData[min_ctr] = q3.readPSI();
          q4.minuteData[min_ctr] = q4.readPSI();
          mainMode();
        }
     
      }
      sampleCount++;
    }
   
  }
  if(sampleCount == 1200) {
    safeSerialPrint("end\n");
  }

}



void refillMode() {
  if(q1.is_refilling) {
    if(q1.readPSI() >= q1.ideal_pressure) {
      q1.is_refilling = false;
      q1.closeValve();
      safeSerialPrint("Closing q1\n");
      for(int i = 0; i < N; i++){
        dequeue(q1.sensorData);
      }
      q1.updateLeakLog();
    }
  }
  if(q2.is_refilling) {
    if(q2.readPSI() >= q2.ideal_pressure) {
      q2.is_refilling = false;
      q2.closeValve();
      safeSerialPrint("Closing q2\n");
      for(int i = 0; i < N; i++){
        dequeue(q2.sensorData);
      }
       q2.updateLeakLog();
    }
  }
  if(q3.is_refilling) {
    if(q3.readPSI() >= q3.ideal_pressure) {
      q3.is_refilling = false;
      q3.closeValve();
      safeSerialPrint("Closing q3\n");
      for(int i = 0; i < N; i++){
        dequeue(q3.sensorData);
      }
       q3.updateLeakLog();
    }
  }
  if(q4.is_refilling) {
    if(q4.readPSI() >= q4.ideal_pressure) {
      q4.is_refilling = false;
      q4.closeValve();
      safeSerialPrint("Closing q4\n");
      for(int i = 0; i < N; i++){
        dequeue(q4.sensorData);
      }
       q4.updateLeakLog();
    }
  }
}


void deflateMode() {
  if(q1.is_deflating) {
    if(q1.readPSI() <= q1.ideal_pressure) {
      q1.is_deflating = false;
      q1.closeValve();
      safeSerialPrint("Closing q1\n");
      for(int i = 0; i < N; i++){
        dequeue(q1.sensorData);
      }
    }
  }
  if(q2.is_deflating) {
    if(q2.readPSI() <= q2.ideal_pressure) {
      q2.is_deflating = false;
      q2.closeValve();
      safeSerialPrint("Closing q2\n");
      for(int i = 0; i < N; i++){
        dequeue(q2.sensorData);
      }
    }
  }
  if(q3.is_deflating) {
    if(q3.readPSI() <= q3.ideal_pressure) {
      q3.is_deflating = false;
      q3.closeValve();
      safeSerialPrint("Closing q3\n");
      for(int i = 0; i < N; i++){
        dequeue(q3.sensorData);
      }
    }
  }
  if(q4.is_deflating) {
    if(q4.readPSI() <= q4.ideal_pressure) {
      q4.is_deflating = false;
      q4.closeValve();
      safeSerialPrint("Closing q4\n");
      for(int i = 0; i < N; i++){
        dequeue(q4.sensorData);
      }
    }
  }
}

void mainMode() {
  min_ctr++;
  if(min_ctr == 60) {
    min_ctr = 0;
    float s1_cur = q1.minAvg();
    float s2_cur = q2.minAvg();
    float s3_cur = q3.minAvg();
    float s4_cur = q4.minAvg();

    enqueue(q1.sensorData, s1_cur);
    enqueue(q2.sensorData, s2_cur);
    enqueue(q3.sensorData, s3_cur);
    enqueue(q4.sensorData, s4_cur);

    safeSerialPrint("\nMINUTE AVERAGES:\n");
    safeSerialPrint("Average for Q1: " + String(s1_cur) + "\n");
    safeSerialPrint("Average for Q2: " + String(s2_cur) + "\n");
    safeSerialPrint("Average for Q3: " + String(s3_cur) + "\n");
    safeSerialPrint("Average for Q4: " + String(s4_cur) + "\n\n");

    if(isFull(q1.sensorData)) {
      // Checking that one quadrant's data queue is full will check that all are full
      if(is_pressure_unacceptable(s1_cur, q1.ideal_pressure)) {
        float moving_avg1 = getMovingAverage(q1.sensorData);
        safeSerialPrint("Moving Average for Q1: " + String(moving_avg1) + "\n");
        if(isAverageUnacceptable(moving_avg1, q1.ideal_pressure)) {
          safeSerialPrint("Opening Q1 Valve\n");
          if (moving_avg1 < q1.ideal_pressure) {
            q1.is_refilling = true;
            q1.openValve();
            pump_on = true;
            digitalWrite(PUMP, HIGH);
            safeSerialPrint("Turning on pump\n");
          } else {
            q1.is_deflating = true;
            q1.openValve();
            exhaust_on = true;
            digitalWrite(VALVE_OUT, HIGH);
            safeSerialPrint("Opening Exhaust valve\n");
          }
       
        }
      }
      if(is_pressure_unacceptable(s2_cur, q2.ideal_pressure)) {
        float moving_avg2 = getMovingAverage(q2.sensorData);
        safeSerialPrint("Moving Average for Q2: " + String(moving_avg2) + "\n");
        if(isAverageUnacceptable(moving_avg2, q2.ideal_pressure)) {
          safeSerialPrint("Opening Q2 Valve\n");
          if (moving_avg2 < q2.ideal_pressure) {
            q2.is_refilling = true;
            q2.openValve();
            pump_on = true;
            digitalWrite(PUMP, HIGH);
          } else {
            q2.is_deflating = true;
            q2.openValve();
            exhaust_on = true;
            digitalWrite(VALVE_OUT, HIGH);
            safeSerialPrint("Opening Exhaust valve\n");
          }
        
        }
      }
      if(is_pressure_unacceptable(s3_cur, q3.ideal_pressure)){
        float moving_avg3 = getMovingAverage(q3.sensorData);
        safeSerialPrint("Moving Average for Q3: " + String(moving_avg3) + "\n");
        if(isAverageUnacceptable(moving_avg3, q3.ideal_pressure)) {
          safeSerialPrint("Opening Q3 Valve\n");
          if (moving_avg3 < q3.ideal_pressure) {
          q3.is_refilling = true;
          q3.openValve();
          pump_on = true;
          digitalWrite(PUMP, HIGH);
        } else {
          q3.is_deflating = true;
          q3.openValve();
          exhaust_on = true;
          digitalWrite(VALVE_OUT, HIGH);
          safeSerialPrint("Opening Exhaust valve\n");
        }
      }
      }
      if(is_pressure_unacceptable(s4_cur, q4.ideal_pressure)){
        float moving_avg4 = getMovingAverage(q4.sensorData);
        safeSerialPrint("Moving Average for Q4: " + String(moving_avg4) + "\n");
        if(isAverageUnacceptable(moving_avg4, q4.ideal_pressure)) {
          safeSerialPrint("Opening Q4 Valve\n");
           if (moving_avg4 < q4.ideal_pressure) {
            q4.is_refilling = true;
            q4.openValve();
            pump_on = true;
            digitalWrite(PUMP, HIGH);
          } else {
            q4.is_deflating = true;
            q4.openValve();
            exhaust_on = true;
            digitalWrite(VALVE_OUT, HIGH);
            safeSerialPrint("Opening Exhaust valve\n");
          }
      }
    }
  }
  }
}


// TO DO: 
// INFLATE ALL FUNCTION:


// DEFLATE ALL FUNCTION:


// SET NEW IDEAL PRESSURES:
void recordAndSetIdealPressures() {
  // Read the current pressure for each quadrant
  float currentPressureQ1 = q1.readPSI();
  float currentPressureQ2 = q2.readPSI();
  float currentPressureQ3 = q3.readPSI();
  float currentPressureQ4 = q4.readPSI();

  // Update the ideal pressure for each quadrant
  q1.ideal_pressure = q1.readPSI();
  q2.ideal_pressure = q2.readPSI();
  q3.ideal_pressure = q3.readPSI();
  q4.ideal_pressure = q4.readPSI();

//  safeSerialPrint("Updated Ideal Pressures:\n");
//  safeSerialPrint("Q1: " + String(currentPressureQ1) + "\n");
//  safeSerialPrint("Q2: " + String(currentPressureQ2) + "\n");
//  safeSerialPrint("Q3: " + String(currentPressureQ3) + "\n");
//  safeSerialPrint("Q4: " + String(currentPressureQ4) + "\n");
}

// DETECT IF SMALL LEAK OCCURED
// - 

// DETECT IF LARGE LEAK OCCURED 
// - 


// Algorithm helper functions
bool is_pressure_unacceptable(float current_pressure, float ideal_pressure){
  return abs(current_pressure - ideal_pressure) > acceptable_threshold;
}
bool isAverageUnacceptable(float moving_average, float ideal_pressure){
  return abs(moving_average - ideal_pressure) > acceptable_threshold && moving_average > -10;
}

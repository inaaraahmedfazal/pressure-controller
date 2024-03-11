#define N 5
#define M 4

//SOLENOID VALVES
const int VALVE1 = 2;
const int VALVE2 = 3;
const int VALVE3 = 4;
const int VALVE4 = 5;
const int VALVE_OUT = 6;
const int PUMP = 7;

const int sensorPin1 = A1;
const int sensorPin2 = A2;
const int sensorPin3 = A3;
const int sensorPin4 = A4;

static unsigned long lastRefreshTime = 0;
const int sampling_interval = 1000;
static unsigned int min_ctr = 0;
const float acceptable_threshold = 0.1;
static unsigned int sampleCount = 0;


const float STD_DEV_MULTIPLIER = 1.5;

bool pump_on = false;
bool exhaust_on = false;

//TO BE SET BY THE UI
enum ActionState {
  MAIN_MODE = 0,
  INFLATE_ALL = 1,
  DEFLATE_ALL = 2,
  SET_NEW_REF = 3,
  INFLATE_QUAD = 4, 
  DEFLATE_QUAD = 5
};

uint8_t actionState = MAIN_MODE;

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
  unsigned int detectionTimestamps[4]; // stores logs for last hour or most recent log if greater than an hour
  int count; // Current number of detections
};

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
        return -1; // Return a special value to indicate an error
    }

    return queue->items[queue->front];
}

// Function to calculate the mean of all values in the queue
float calculateMean(struct Queue* queue) {
    if (isEmpty(queue)) {
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
  struct LeakLog leak_log;

  public:
    bool hasSmallLeak;
    bool hasLargeLeak;
    bool is_refilling;
    bool is_deflating;
    float ideal_pressure;
    float minuteSum;
    struct Queue* sensorData;
  Quadrant(int n, int sp, int vp, float m, float b, float ip) {
    num = n;
    sensor_pin = sp;
    valve_pin = vp;
    calibration_m = m;
    calibration_b = b;
    hasSmallLeak = false;
    hasLargeLeak = false;
    is_refilling = false;
    is_deflating = false;
    ideal_pressure = ip;
    minuteSum = 0;
    initializeQueue(sensorData, N);
    for(int i = 0; i < 4; i++) {
      leak_log.detectionTimestamps[i] = 0;
    }
    leak_log.count = 0; 
  }
  ~Quadrant() {
    destroyQueue(sensorData);
    sensorData = NULL;
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
    //safeSerialPrint(String("Pressure PSI for Quadrant " + String(this->num) + ": " + String(pressureInPSI) + "\n"));
    return pressureInPSI;
  }

  float minAvg(){
    float avg = minuteSum / 60.0;
    minuteSum = 0;
    return avg;
  }

  void updateLeakLog() {
    unsigned long currentTime = millis();
    unsigned int currentMinutes = currentTime / 60000; // Convert current time to minutes

    // Remove entries older than 1 hour, except the most recent if all are old
    int recentIndex = -1; // Track the index of the first recent entry within the last hour
    for (int i = leak_log.count - 1; i >= 0; i--) {
        unsigned int minutesAgo = currentMinutes - leak_log.detectionTimestamps[i];
        if (minutesAgo <= 60) {
            recentIndex = i; // Found a recent entry within the last hour
            break; // Stop at the first recent entry from the end
        }
    }

    if (recentIndex == -1) { // If no entries are within the last hour
        // Keep only the most recent entry, if any exist
        if (leak_log.count > 0) {
            leak_log.detectionTimestamps[0] = leak_log.detectionTimestamps[leak_log.count - 1];
            leak_log.count = 1;
        }
    } else if (recentIndex > 0) {
        // Shift recent entries to the beginning of the array
        int shiftCount = recentIndex;
        for (int i = 0; i + shiftCount < leak_log.count; i++) {
            leak_log.detectionTimestamps[i] = leak_log.detectionTimestamps[i + shiftCount];
        }
        leak_log.count -= shiftCount;
    }

    // Add the new leak detection time if it's not the same minute as the last logged time
    if (leak_log.count == 0 || currentMinutes != leak_log.detectionTimestamps[leak_log.count - 1]) {
        if (leak_log.count < sizeof(leak_log.detectionTimestamps) / sizeof(leak_log.detectionTimestamps[0])) {
            leak_log.detectionTimestamps[leak_log.count++] = currentMinutes;
        }
    }
  }

  void evaluateLeakSeverity() {
      unsigned int currentMinutes = millis() / 60000; // Get current time in minutes
      int countLastHour = 0;
      bool hasActuationLast24Hours = false;
    
      for(int i = 0; i < leak_log.count; i++) {
        unsigned int minutesAgo = currentMinutes - leak_log.detectionTimestamps[i];
    
        if (minutesAgo <= 60) { // Checks if within the last hour
          countLastHour++;
        }
        if (minutesAgo <= 1440) { // Checks if within the last 24 hours
          hasActuationLast24Hours = true;
        }
      }
  
      if (countLastHour > 2) {
        hasLargeLeak = true;
        hasSmallLeak = false;
      } else if(hasActuationLast24Hours) {
        hasSmallLeak = true;
        hasLargeLeak = false;
      } else {
        hasSmallLeak = false;
        hasLargeLeak = false;
      }
    }
  
};

//CALIBRATION - set the initial values 
Quadrant* q1;
Quadrant* q2;
Quadrant* q3;
Quadrant* q4;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  // Initialize serial communication
  pinMode(VALVE1, OUTPUT); 
  pinMode(VALVE2, OUTPUT); 
  pinMode(VALVE3, OUTPUT); 
  pinMode(VALVE4, OUTPUT); 
  
  pinMode(PUMP, OUTPUT); 
  pinMode(VALVE_OUT, OUTPUT);
  pinMode(sensorPin1, INPUT);
  pinMode(sensorPin2, INPUT);
  pinMode(sensorPin3, INPUT);
  pinMode(sensorPin4, INPUT);
  actionState = SET_NEW_REF;

  q1 = new Quadrant(1, sensorPin1, VALVE1, 0.00792, -0.252, 0.54);
  q2 = new Quadrant(2, sensorPin2, VALVE2, 0.00789, -0.302, 0.59);
  q3 = new Quadrant(3, sensorPin3, VALVE3, 0.00792, -0.281, 0.70);
  q4 = new Quadrant(4, sensorPin4, VALVE4, 0.00789, -0.262, 0.20);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(sampleCount < 6000) {
   
    if(millis() - lastRefreshTime >= sampling_interval) {
      // Serial.println("sampling");
      //Serial.println("\n\nStarting sample:\n");
      lastRefreshTime = millis();

      // currently setting state here - can set this else-where 
      // ALSO set logic for setting reading state from UI here 
      if(q1->is_refilling || q2->is_refilling || q3->is_refilling || q4->is_refilling){
        actionState = INFLATE_QUAD;
      }
      else if(q1->is_deflating || q2->is_deflating || q3->is_deflating || q4->is_deflating){
        actionState = DEFLATE_QUAD;
      } 
      
      switch (actionState) {
        case INFLATE_ALL:
          inflateAllMode(); 
          break;
        case DEFLATE_ALL:
          deflateAllMode();
          break;
        case SET_NEW_REF:
          Serial.println("Reached Case");
          recordAndSetIdealPressures();
          break;  
        case INFLATE_QUAD:
          refillMode();
          break;
        case DEFLATE_QUAD:
          deflateMode();
          break;
        case MAIN_MODE: 
          if(pump_on) {
                digitalWrite(PUMP, LOW);
                pump_on = false;
            }
            if (exhaust_on) {
              digitalWrite(VALVE_OUT, LOW);
              exhaust_on = false;
            }
            q1->minuteSum += q1->readPSI();
            q2->minuteSum += q2->readPSI();
            q3->minuteSum += q3->readPSI();
            q4->minuteSum += q4->readPSI();
            mainMode();
            break;
        default:
            if(pump_on) {
                digitalWrite(PUMP, LOW);
                pump_on = false;
            }
            if (exhaust_on) {
              digitalWrite(VALVE_OUT, LOW);
              exhaust_on = false;
            }
            q1->minuteSum += q1->readPSI();
            q2->minuteSum += q2->readPSI();
            q3->minuteSum += q3->readPSI();
            q4->minuteSum += q4->readPSI();
            mainMode();
            break;
      }

      sampleCount++;
      if(sampleCount == 1200) {
        Serial.print("end\n");
        delete q1;
        delete q2;
        delete q3;
        delete q4;
      }
    } // end if millis - refreshtime
  } // end if samplecount < 1200


} // end loop
 

void refillMode() {
  if(q1->is_refilling) {
    if(q1->readPSI() >= q1->ideal_pressure) {
      q1->is_refilling = false;
      q1->closeValve();
      Serial.print("Closing q1\n");
      for(int i = 0; i < N; i++){
        dequeue(q1->sensorData);
      }
      q1->updateLeakLog();
    }
  }
  if(q2->is_refilling) {
    if(q2->readPSI() >= q2->ideal_pressure) {
      q2->is_refilling = false;
      q2->closeValve();
      Serial.print("Closing q2\n");
      for(int i = 0; i < N; i++){
        dequeue(q2->sensorData);
      }
       q2->updateLeakLog();
    }
  }
  if(q3->is_refilling) {
    if(q3->readPSI() >= q3->ideal_pressure) {
      q3->is_refilling = false;
      q3->closeValve();
      Serial.print("Closing q3\n");
      for(int i = 0; i < N; i++){
        dequeue(q3->sensorData);
      }
       q3->updateLeakLog();
    }
  }
  if(q4->is_refilling) {
    if(q4->readPSI() >= q4->ideal_pressure) {
      q4->is_refilling = false;
      q4->closeValve();
      Serial.print("Closing q4\n");
      for(int i = 0; i < N; i++){
        dequeue(q4->sensorData);
      }
       q4->updateLeakLog();
    }
  }
}


void deflateMode() {
  if(q1->is_deflating) {
    if(q1->readPSI() <= q1->ideal_pressure) {
      q1->is_deflating = false;
      q1->closeValve();
      Serial.print("Closing q1\n");
      for(int i = 0; i < N; i++){
        dequeue(q1->sensorData);
      }
    }
  }
  if(q2->is_deflating) {
    if(q2->readPSI() <= q2->ideal_pressure) {
      q2->is_deflating = false;
      q2->closeValve();
      Serial.print("Closing q2\n");
      for(int i = 0; i < N; i++){
        dequeue(q2->sensorData);
      }
    }
  }
  if(q3->is_deflating) {
    if(q3->readPSI() <= q3->ideal_pressure) {
      q3->is_deflating = false;
      q3->closeValve();
      Serial.print("Closing q3\n");
      for(int i = 0; i < N; i++){
        dequeue(q3->sensorData);
      }
    }
  }
  if(q4->is_deflating) {
    if(q4->readPSI() <= q4->ideal_pressure) {
      q4->is_deflating = false;
      q4->closeValve();
      Serial.print("Closing q4\n");
      for(int i = 0; i < N; i++){
        dequeue(q4->sensorData);
      }
    }
  }
}

void mainMode() {
  min_ctr++;
  //safeSerialPrint(String(min_ctr));
  if(min_ctr == 60) {
    min_ctr = 0;
    float s1_cur = q1->minAvg();
    float s2_cur = q2->minAvg();
    float s3_cur = q3->minAvg();
    float s4_cur = q4->minAvg();

    enqueue(q1->sensorData, s1_cur);
    enqueue(q2->sensorData, s2_cur);
    enqueue(q3->sensorData, s3_cur);
    enqueue(q4->sensorData, s4_cur);

    Serial.print("\nMINUTE AVERAGES:\n");
    Serial.print("Average for Q1: " + String(s1_cur) + "\n");
    Serial.print("Average for Q2: " + String(s2_cur) + "\n");
    Serial.print("Average for Q3: " + String(s3_cur) + "\n");
    Serial.print("Average for Q4: " + String(s4_cur) + "\n\n");

    if(isFull(q1->sensorData)) {
      // Checking that one quadrant's data queue is full will check that all are full
      if(is_pressure_unacceptable(s1_cur, q1->ideal_pressure)) {
        float moving_avg1 = getMovingAverage(q1->sensorData);
        Serial.print("Moving Average for Q1: " + String(moving_avg1) + "\n");
        if(isAverageUnacceptable(moving_avg1, q1->ideal_pressure)) {
          Serial.print("Opening Q1 Valve\n");
          if (moving_avg1 < q1->ideal_pressure) {
            q1->is_refilling = true;
            q1->openValve();
            pump_on = true;
            digitalWrite(PUMP, HIGH);
            Serial.print("Turning on pump\n");
          } else {
            q1->is_deflating = true;
            q1->openValve();
            exhaust_on = true;
            digitalWrite(VALVE_OUT, HIGH);
            Serial.print("Opening Exhaust valve\n");
          }
       
        }
      }
      if(is_pressure_unacceptable(s2_cur, q2->ideal_pressure)) {
        float moving_avg2 = getMovingAverage(q2->sensorData);
        Serial.print("Moving Average for Q2: " + String(moving_avg2) + "\n");
        if(isAverageUnacceptable(moving_avg2, q2->ideal_pressure)) {
          Serial.print("Opening Q2 Valve\n");
          if (moving_avg2 < q2->ideal_pressure) {
            q2->is_refilling = true;
            q2->openValve();
            pump_on = true;
            digitalWrite(PUMP, HIGH);
          } else {
            q2->is_deflating = true;
            q2->openValve();
            exhaust_on = true;
            digitalWrite(VALVE_OUT, HIGH);
            Serial.print("Opening Exhaust valve\n");
          }
        
        }
      }
      if(is_pressure_unacceptable(s3_cur, q3->ideal_pressure)){
        float moving_avg3 = getMovingAverage(q3->sensorData);
        Serial.print("Moving Average for Q3: " + String(moving_avg3) + "\n");
        if(isAverageUnacceptable(moving_avg3, q3->ideal_pressure)) {
          Serial.print("Opening Q3 Valve\n");
          if (moving_avg3 < q3->ideal_pressure) {
          q3->is_refilling = true;
          q3->openValve();
          pump_on = true;
          digitalWrite(PUMP, HIGH);
        } else {
          q3->is_deflating = true;
          q3->openValve();
          exhaust_on = true;
          digitalWrite(VALVE_OUT, HIGH);
          Serial.print("Opening Exhaust valve\n");
        }
      }
      }
      if(is_pressure_unacceptable(s4_cur, q4->ideal_pressure)){
        float moving_avg4 = getMovingAverage(q4->sensorData);
        Serial.print("Moving Average for Q4: " + String(moving_avg4) + "\n");
        if(isAverageUnacceptable(moving_avg4, q4->ideal_pressure)) {
          Serial.print("Opening Q4 Valve\n");
           if (moving_avg4 < q4->ideal_pressure) {
            q4->is_refilling = true;
            q4->openValve();
            pump_on = true;
            digitalWrite(PUMP, HIGH);
          } else {
            q4->is_deflating = true;
            q4->openValve();
            exhaust_on = true;
            digitalWrite(VALVE_OUT, HIGH);
            Serial.print("Opening Exhaust valve\n");
          }
      }
    }
  }
  }
  q1->evaluateLeakSeverity();
  q2->evaluateLeakSeverity();
  q3->evaluateLeakSeverity();
  q4->evaluateLeakSeverity();
  
}


// FUNCTIONS FOR SET-MODE -----------------------------------------

void deflateAllMode() {
  Serial.print("Deflating All");
  digitalWrite(VALVE1, HIGH);
  digitalWrite(VALVE2, HIGH);
  digitalWrite(VALVE3, HIGH);
  digitalWrite(VALVE4, HIGH);
  digitalWrite(VALVE_OUT, HIGH);
  digitalWrite(PUMP, LOW);
}

// DEFLATE ALL FUNCTION:
void inflateAllMode() {
  Serial.print("Inflating All");
  digitalWrite(VALVE1, HIGH);
  digitalWrite(VALVE2, HIGH);
  digitalWrite(VALVE3, HIGH);
  digitalWrite(VALVE4, HIGH);
  digitalWrite(VALVE_OUT, LOW);
  digitalWrite(PUMP, HIGH);
}

// SET NEW IDEAL PRESSURES:
void recordAndSetIdealPressures() {
  Serial.println("Reached set");
  // Read the current pressure for each quadrant
  float currentPressureQ1 = q1->readPSI();
  float currentPressureQ2 = q2->readPSI();
  float currentPressureQ3 = q3->readPSI();
  float currentPressureQ4 = q4->readPSI();

  // Update the ideal pressure for each quadrant
  q1->ideal_pressure = currentPressureQ1;
  q2->ideal_pressure = currentPressureQ2;
  q3->ideal_pressure = currentPressureQ3;
  q4->ideal_pressure = currentPressureQ4;

  Serial.print("Updated Ideal Pressures:\n");
  Serial.print("Q1: " + String(currentPressureQ1) + "\n");
  Serial.print("Q2: " + String(currentPressureQ2) + "\n");
  Serial.print("Q3: " + String(currentPressureQ3) + "\n");
  Serial.print("Q4: " + String(currentPressureQ4) + "\n");
  actionState = MAIN_MODE;
}



// Algorithm helper functions
bool is_pressure_unacceptable(float current_pressure, float ideal_pressure){
  return abs(current_pressure - ideal_pressure) > acceptable_threshold;
}
bool isAverageUnacceptable(float moving_average, float ideal_pressure){
  return abs(moving_average - ideal_pressure) > acceptable_threshold && moving_average > -10;
}

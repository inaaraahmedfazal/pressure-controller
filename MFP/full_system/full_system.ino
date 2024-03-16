#define N 15
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
const float acceptable_threshold = 0.1; //PSI
const float residual_err_thres = 0.1; //PSI
static unsigned int sampleCount = 0;

const float STD_DEV_MULTIPLIER = 1.5;

bool pump_on = false;
bool exhaust_on = false;


const int MAX_REFILL_TIME = 30000;
const int MIN_REFILL_TIME = 5000;
const float MAX_PSI_DIFF = 0.4;
const float MIN_PSI_DIFF = 0.01;
const float REFILL_EQ_M = (MAX_REFILL_TIME - MIN_REFILL_TIME)/(MAX_PSI_DIFF - MIN_PSI_DIFF);
const float REFILL_EQ_B = (MAX_PSI_DIFF*MIN_REFILL_TIME - MIN_PSI_DIFF*MAX_REFILL_TIME)/(MAX_PSI_DIFF - MIN_PSI_DIFF);
const unsigned long REFILL_MODE_LIMIT = 240000;
  
//TO BE SET BY THE UI
enum ActionState {
  MAIN_MODE = 0,
  INFLATE_ALL = 1,
  DEFLATE_ALL = 2,
  SET_NEW_REF = 3,
  INFLATE_QUAD = 4, 
  DEFLATE_QUAD = 5
};
ActionState actionState = DEFLATE_ALL;

struct Queue {
  int front, rear;
  int maxSize;
  float* items;
};

struct LeakLog {
  unsigned int detectionTimestamps[4]; // stores logs for last hour or most recent log if greater than an hour
  int count; // Current number of detections
};


// Function to calculate the standard deviation of all values in the queue
float calculateStdDev(struct Queue* queue) {
  if (isEmpty(queue)) {
    //printf("Queue is empty. Cannot calculate standard deviation.\n");
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
      //printf("Queue is empty. Cannot calculate mean.\n");
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

   void refillRoutine() {
    unsigned long refillStartTime = millis();
    unsigned long curTime = millis();
    while(curTime - refillStartTime < REFILL_MODE_LIMIT && is_refilling == true) {
      delay(3000);
      float curPSI = readPSI();
      float absDifference = abs(curPSI - ideal_pressure);
      Serial.print("abs diff:  ");
      Serial.println(absDifference);
      if(curPSI >= ideal_pressure || absDifference < 0.01) {
        Serial.println("stop refilling");
        is_refilling = false;
      } else {
        int delayTime = 0;
        if(absDifference > 0.4){
          delayTime = MAX_REFILL_TIME;
        }
        else {
          delayTime = int(REFILL_EQ_M * absDifference + REFILL_EQ_B);
          Serial.print("delay time:  ");
          Serial.println(delayTime);
        }
        openValve();
        digitalWrite(PUMP, HIGH);
        delay(delayTime);
        digitalWrite(PUMP, LOW);
        closeValve();
      }
      curTime = millis();
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
  // Read the current pressure for each quadrant
  float currentPressureQ1 = q1->readPSI();
  float currentPressureQ2 = q2->readPSI();
  float currentPressureQ3 = q3->readPSI();
  float currentPressureQ4 = q4->readPSI();

  // Update the ideal pressure for each quadrant
  q1->ideal_pressure = q1->readPSI();
  q2->ideal_pressure = q2->readPSI();
  q3->ideal_pressure = q3->readPSI();
  q4->ideal_pressure = q4->readPSI();

  Serial.println("Updated Ideal Pressures:\n");
  Serial.println("Q1: " + String(currentPressureQ1) + "\n");
  Serial.println("Q2: " + String(currentPressureQ2) + "\n");
  Serial.println("Q3: " + String(currentPressureQ3) + "\n");
  Serial.println("Q4: " + String(currentPressureQ4) + "\n");
  actionState = MAIN_MODE;
}


float mean(float arr[], int n){
  float sum = 0.0;
  for (int i = 0; i < n; i++) {
    sum += arr[i];
  }
  return sum / n;
}

float get_slope(float x[], float y[], int n){
  float sum_xy = 0.0;
  float sum_x_squared = 0.0;

  float x_mean = mean(x, n);
  float y_mean = mean(y, n);

  for (int i = 0; i < n; i++) {
    sum_xy += (x[i] - x_mean) * (y[i] - y_mean);
    sum_x_squared += (x[i] - x_mean) * (x[i] - x_mean);
  }

  return sum_xy / sum_x_squared;
}


float get_intercept(float x[], float y[], int n, float slope){
  float x_mean = mean(x, n);
  float y_mean = mean(y, n);
  return y_mean - slope * x_mean;
}

bool is_datapoint_an_outlier(struct Queue* queue, float new_data){
  float mean = calculateMean(queue);
  float std = calculateStdDev(queue);
  int ctr = 0;
  for(int i = 0; i < N; i++){
    if(abs(queue->items[i] - mean) <= STD_DEV_MULTIPLIER * std){
      ctr++;
    }
  }
  float filtered_data[ctr - 1];
  float filtered_indices[ctr - 1];
  int j = 0;
  for(int i = 0; i < N-1; i++){
    if(abs(queue->items[i] - mean) <= STD_DEV_MULTIPLIER * std){
      filtered_data[j] = queue->items[i];
      filtered_indices[j] = i;
      j++;
    }
   }
  float slope = get_slope(filtered_indices, filtered_data, ctr - 1);
  float intercept = get_intercept(filtered_indices, filtered_data, ctr - 1, slope);

  float predicted_y = slope * (N-1) + intercept;
  float residual_error = abs(predicted_y - queue->items[N-1]);

  return residual_error > residual_err_thres;
  
}

void refillMode() {
   Serial.println("refill Mode");
  if(q1->is_refilling) {
    q1->refillRoutine();
    //safeSerialPrint(String("Closing q1\n"));
    for(int i = 0; i < N; i++){
      Serial.println("dequeing"); 
      dequeue(q1->sensorData);
    }
    q1->updateLeakLog();
  }
  if(q2->is_refilling) {
  q2->refillRoutine();
    //safeSerialPrint(String("Closing q2\n"));
    for(int i = 0; i < N; i++){
      dequeue(q2->sensorData);
    }
    q2->updateLeakLog();
  }
  if(q3->is_refilling) {
    Serial.println("Should enter refill routine");
    q3->refillRoutine();

    for(int i = 0; i < N; i++){
      dequeue(q3->sensorData);
    }
    q3->updateLeakLog();
  }
  if(q4->is_refilling) {
  q4->refillRoutine();
    //safeSerialPrint(String("Closing q4\n"));
    for(int i = 0; i < N; i++){
      dequeue(q4->sensorData);
    }
    q4->updateLeakLog();
  }
} // end refill mode 


void deflateMode() {
  if(q1->is_deflating) {
    if(q1->readPSI() <= q1->ideal_pressure) {
      q1->is_deflating = false;
      q1->closeValve();
      //safeSerialPrint(String("Closing q1\n"));
      for(int i = 0; i < N; i++){
        dequeue(q1->sensorData);
      }
    }
  }
  if(q2->is_deflating) {
    
    if(q2->readPSI() <= q2->ideal_pressure) {
      q2->is_deflating = false;
      q2->closeValve();
      //safeSerialPrint(String("Closing q2\n"));
      for(int i = 0; i < N; i++){
        dequeue(q2->sensorData);
      }
    }
  }
  if(q3->is_deflating) {
    if(q3->readPSI() <= q3->ideal_pressure) {
      q3->is_deflating = false;
      q3->closeValve();
      //safeSerialPrint(String("Closing q3\n"));
      for(int i = 0; i < N; i++){
        dequeue(q3->sensorData);
      }
    }
  }
  if(q4->is_deflating) {
    if(q4->readPSI() <= q4->ideal_pressure) {
      q4->is_deflating = false;
      q4->closeValve();
      //safeSerialPrint(String("Closing q4\n"));
      for(int i = 0; i < N; i++){
        dequeue(q4->sensorData);
      }
    }
  }
}



void pressureCheck(class Quadrant* q, float curr_min_avg) {

  //float moving_avg = getMovingAverage(q->sensorData); Serial.println("Moving Average: " + String(moving_avg) + "\n");
// ^maybe use this for the has been seated check where we would check if this avg > 0.01 or something
// may need to add extra checks for is seated logic individual to the quadrant
  // if 15 min worth of data, and current point outside ideal range, and curr point is not an outlier (consistent trend)
//  Serial.print("isFull: ");
//  Serial.println(isFull(q->sensorData));
//  Serial.print("is unacceptable: ");
//  Serial.println(is_pressure_unacceptable(curr_min_avg, q->ideal_pressure));
//  Serial.print("is outlier: ");
//  Serial.println(is_datapoint_an_outlier(q->sensorData, curr_min_avg));
//  Serial.println(" ");
  
  if(isFull(q->sensorData)&& is_pressure_unacceptable(curr_min_avg, q->ideal_pressure)
    && !is_datapoint_an_outlier(q->sensorData, curr_min_avg)){
      Serial.println("checking quad");
      if (curr_min_avg < q->ideal_pressure) {
        Serial.println("set to refill");
        q->is_refilling = true;
      } else {
        Serial.println("set to deflate");
        q->is_deflating = true;
        q->openValve();
      }
   }
} // end checkQuad

void mainMode() {
  min_ctr++;
  if(min_ctr == 60) {
    min_ctr = 0;
    float s1_curr = q1->minAvg();
    float s2_curr = q2->minAvg();
    float s3_curr = q3->minAvg();
    float s4_curr = q4->minAvg();

    enqueue(q1->sensorData, s1_curr);
    enqueue(q2->sensorData, s2_curr);
    enqueue(q3->sensorData, s3_curr);
    enqueue(q4->sensorData, s4_curr);

    Serial.println("\nMINUTE AVERAGES:\n");
    Serial.println("Average for Q1: " + String(s1_curr) + "\n");
    Serial.println("Average for Q2: " + String(s2_curr) + "\n");
    Serial.println("Average for Q3: " + String(s3_curr) + "\n");
    Serial.println("Average for Q4: " + String(s4_curr) + "\n\n");
    
    if (user_is_seated(s1_curr, s2_curr, s3_curr, s4_curr)){
      pressureCheck(q1, s1_curr);
      pressureCheck(q2, s2_curr);
      pressureCheck(q3, s3_curr);
      pressureCheck(q4, s4_curr);
    }
    q1->evaluateLeakSeverity();
    q2->evaluateLeakSeverity();
    q3->evaluateLeakSeverity();
    q4->evaluateLeakSeverity();
  } // end if a minute is reached
}// end main mode


// Algorithm helper functions
bool is_pressure_unacceptable(float current_pressure, float ideal_pressure){
  return abs(current_pressure - ideal_pressure) > acceptable_threshold;
}

bool user_is_seated(float q1_curr, float q2_curr, float q3_curr, float q4_curr) {
  // Initialize a counter for the number of floats less than 0.1
  int count_less_than_seated = 0;
  
  // Check each float and increment the counter if it's less than 0.1
  if (q1_curr < 0.03) count_less_than_seated++;
  if (q2_curr < 0.03) count_less_than_seated++;
  if (q3_curr < 0.03) count_less_than_seated++;
  if (q4_curr < 0.03) count_less_than_seated++;

  // If three or more floats are less than 0.1, return false
  if (count_less_than_seated > 1) {
    //Serial.println("user is detected to be not seated");
    return false;
  } else {
    Serial.println("user is detected to be seated");
    return true; // Otherwise, return true
  }
}



void loop() {

  if(sampleCount < 1200) {
    if(millis() - lastRefreshTime >= sampling_interval) {
      lastRefreshTime = millis();
       if(q1->is_refilling || q2->is_refilling || q3->is_refilling || q4->is_refilling){
         Serial.println("action state set to inflate");
         actionState = INFLATE_QUAD;
        } else if(q1->is_deflating || q2->is_deflating || q3->is_deflating || q4->is_deflating){
          Serial.println("action state set to deflate");
          actionState = DEFLATE_QUAD;
        } 
        else if (actionState != SET_NEW_REF) {
          actionState = MAIN_MODE;
        }
        
    
       switch (actionState) {
          case INFLATE_ALL:
            inflateAllMode(); 
            break;
          case DEFLATE_ALL:
            deflateAllMode();
            break;
          case SET_NEW_REF:
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
        } // end switch action state 

      sampleCount++;
      if(sampleCount == 1200) {
        Serial.println(String("end\n"));
        delete q1;
        delete q2;
        delete q3;
        delete q4;
      }
  
    }// end if millis - refreshtime
  } // end if samplecount < 1200
} // end loop 


// QUEUE HELPER FUNCTIONS: ======================================================================================

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
        // If the queue is full, dequeue the front element
        dequeue(queue);
    }
    
    // Enqueue the new element at the rear
    if (isEmpty(queue)) {
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
        //printf("Queue is empty. Cannot dequeue\n");
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
        //printf("Queue is empty\n");
        return -1; // Return a special value to indicate an error
    }

    return queue->items[queue->front];
}

// Function to calculate the mean of all values in the queue
float calculateMean(struct Queue* queue) {
    if (isEmpty(queue)) {
        //printf("Queue is empty. Cannot calculate mean.\n");
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

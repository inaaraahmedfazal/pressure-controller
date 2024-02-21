#define N 15
#define M 4

//SOLENOID VALVES
const int RELAY1 = 4;
const int RELAY2 = 5;
const int RELAY3 = 6;
const int RELAY4 = 7;
const int RELAY5 = 8;
const int RELAY_PUMP = 9;

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
bool isRefilling = false;
float ideal_pressure = 0.6;
bool pump_on = false;

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
  int relay_pin;
  
  public:
  bool is_refilling;
  float minuteData[60];
  struct Queue* sensorData;
  struct Queue* actuationsHistory;
  Quadrant(int n, int sp, int rp, float m, float b) {
    num = n;
    sensor_pin = sp;
    relay_pin = rp;
    calibration_m = m;
    calibration_b = b;
    is_refilling = false;
    initializeQueue(sensorData, N);
    initializeQueue(actuationsHistory, M);
  }
  void openValve(){
    digitalWrite(relay_pin, LOW);
  }
  void closeValve(){
    digitalWrite(relay_pin, HIGH);
  }
  float readPSI(){
    // Read the analog voltage from the pressure transducer
    int sensorPressure = analogRead(this->sensor_pin);
    float pressureInPSI = this->calibration_m*sensorPressure + this->calibration_b;
    Serial.print("Pressure PSI for Quadrant ");
    Serial.print(this->num);
    Serial.print(": ");
    Serial.println(pressureInPSI);

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
};

Quadrant q1(1, sensorPin1, RELAY1, 0.00792, -0.252);
Quadrant q2(2, sensorPin2, RELAY2, 0.00789, -0.302);
Quadrant q3(3, sensorPin3, RELAY3, 0.00792, -0.281);
Quadrant q4(4, sensorPin4, RELAY4, 0.00789, -0.262);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  // Initialize serial communication
  pinMode(RELAY1, OUTPUT); // Set the relay pin as an output
  pinMode(RELAY2, OUTPUT); // Set the relay pin as an output
  pinMode(RELAY3, OUTPUT); // Set the relay pin as an output
  pinMode(RELAY4, OUTPUT); // Set the relay pin as an output
  pinMode(RELAY5, OUTPUT); // Set the relay pin as an output
}

void loop() {
  // put your main code here, to run repeatedly:
  if(sampleCount < 1200) {
    if(millis() - lastRefreshTime >= sampling_interval) {
      Serial.println("Starting sample:");
      lastRefreshTime = millis();

      if(q1.is_refilling || q2.is_refilling || q3.is_refilling || q4.is_refilling) {
        refillMode();
      }
      else {
        if(pump_on) {
          digitalWrite(RELAY_PUMP, LOW);
          pump_on = false;
        }

        q1.minuteData[min_ctr] = q1.readPSI();
        q2.minuteData[min_ctr] = q2.readPSI();
        q3.minuteData[min_ctr] = q3.readPSI();
        q4.minuteData[min_ctr] = q4.readPSI();
        mainMode();
      }


    }
    sampleCount++;
    if(sampleCount == 1200) {
      Serial.println("end");
    }
  }

}

void refillMode() {
  if(q1.is_refilling) {
    if(q1.readPSI() >= ideal_pressure) {
      q1.is_refilling = false;
      q1.closeValve();
      for(int i = 0; i < N; i++){
        dequeue(q1.sensorData);
      }
    }
  }
  if(q2.is_refilling) {
    if(q2.readPSI() >= ideal_pressure) {
      q2.is_refilling = false;
      q2.closeValve();
      for(int i = 0; i < N; i++){
        dequeue(q2.sensorData);
      }
    }
  }
  if(q3.is_refilling) {
    if(q3.readPSI() >= ideal_pressure) {
      q3.is_refilling = false;
      q3.closeValve();
      for(int i = 0; i < N; i++){
        dequeue(q3.sensorData);
      }
    }
  }
  if(q4.is_refilling) {
    if(q4.readPSI() >= ideal_pressure) {
      q4.is_refilling = false;
      q4.closeValve();
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

    Serial.println();
    Serial.println("MINUTE AVERAGES:");
    Serial.print("Average for Q1: ");
    Serial.println(s1_cur);
    Serial.print("Average for Q2: ");
    Serial.println(s2_cur);
    Serial.print("Average for Q3: ");
    Serial.println(s3_cur);
    Serial.print("Average for Q4: ");
    Serial.println(s4_cur);
    Serial.println();

    if(isFull(q1.sensorData)) {
      // Checking that one quadrant's data queue is full will check that all are full
      if(is_pressure_unacceptable(s1_cur)) {
        float moving_avg1 = getMovingAverage(q1.sensorData);
        Serial.print("Moving Average for Q1:");
        Serial.println(moving_avg1);
        if(isAverageUnacceptable(moving_avg1)) {
          Serial.println("Opening Q1 Valve");
          q1.is_refilling = true;
          q1.openValve();
          pump_on = true;
          digitalWrite(RELAY_PUMP, HIGH);
        }
      }
      if(is_pressure_unacceptable(s2_cur)) {
        float moving_avg2 = getMovingAverage(q2.sensorData);
        Serial.print("Moving Average for Q2:");
        Serial.println(moving_avg2);
        if(isAverageUnacceptable(moving_avg2)) {
          Serial.println("Opening Q2 Valve");
          q2.is_refilling = true;
          q2.openValve();
          pump_on = true;
          digitalWrite(RELAY_PUMP, HIGH);
        }
      }
      if(is_pressure_unacceptable(s3_cur)){
        float moving_avg3 = getMovingAverage(q3.sensorData);
        Serial.print("Moving Average for Q3:");
        Serial.println(moving_avg3);
        if(isAverageUnacceptable(moving_avg3)) {
          Serial.println("Opening Q3 Valve");
          q3.is_refilling = true;
          q3.openValve();
          pump_on = true;
          digitalWrite(RELAY_PUMP, HIGH);
        }
      }
      if(is_pressure_unacceptable(s4_cur)){
        float moving_avg4 = getMovingAverage(q4.sensorData);
        Serial.print("Moving Average for Q4:");
        Serial.println(moving_avg4);
        if(isAverageUnacceptable(moving_avg4)) {
          Serial.println("Opening Q4 Valve");
          q4.is_refilling = true;
          q4.openValve();
          pump_on = true;
          digitalWrite(RELAY_PUMP, HIGH);
        }
      }
    }
  }
}

// Algorithm helper functions
bool is_pressure_unacceptable(float current_pressure){
  return abs(current_pressure - ideal_pressure) > acceptable_threshold;
}
bool isAverageUnacceptable(float moving_average){
  return abs(moving_average - ideal_pressure) > acceptable_threshold && moving_average > -10;
}
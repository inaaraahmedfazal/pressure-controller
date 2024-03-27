#define N 5
#define M 4

//SOLENOID VALVES
const int VALVE1 = 2;
const int VALVE2 = 3;
const int VALVE3 = 4;
const int VALVE4 = 5;
const int VALVE_OUT = 6;
const int PUMP = 7;

const int sensorPin1 = A0;
const int sensorPin2 = A1;
const int sensorPin3 = A2;
const int sensorPin4 = A3;

static unsigned long lastRefreshTime = 0;
const int sampling_interval = 1000;
static unsigned int sampleCount = 0;

struct Queue {
  int front, rear;
  int maxSize;
  float* items;
};
struct LeakLog {
  int detectionTimestamps[4]; // stores logs for last hour or most recent log if greater than an hour
  int count; // Current number of detections
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
  
  public:
    bool hasSmallLeak;
    bool hasLargeLeak;
    bool is_refilling;
    bool is_deflating;
    float ideal_pressure;
    float minuteSum;
    struct Queue* sensorData;
    struct LeakLog leak_log;

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
    for(unsigned int i = 0; i < 4; i++) {
      leak_log.detectionTimestamps[i] = 0;
    }
    leak_log.count = 0; 
  }
  ~Quadrant() {
    destroyQueue(sensorData);
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
    Serial.println("Pressure PSI for Quadrant " + String(this->num) + ": " + String(pressureInPSI) + "\n");
    return pressureInPSI;
  }

  float minAvg(){
    float avg = minuteSum / 60.0;
    minuteSum = 0;
    return avg;
  }
  
};

//CALIBRATION - set the initial values 
Quadrant* q1;

void setup() {
  // put your setup code here, to run once:
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

  q1 = new Quadrant(1, sensorPin1, VALVE1, 0.00792, -0.252, 0.54);
}

void loop() {
  if(sampleCount < 1200) {
    if(millis() - lastRefreshTime >= sampling_interval) {
      lastRefreshTime = millis();
      sampleCount++;
      Serial.println(sampleCount);
    }
  }
}

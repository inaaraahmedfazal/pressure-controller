
#include <SoftwareSerial.h>
const int RX = 8; //green cord
const int TX = 9; //orange cord
SoftwareSerial ccSerial = SoftwareSerial(RX, TX); // RX, TX

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
  IDLE_STATE = 0,
  UNLOCK_QUAD = 1, 
  INFLATE_ALL = 2,
  DEFLATE_ALL = 3,
  SET_NEW_REF = 4,
  INFLATE_QUAD = 5, 
  DEFLATE_QUAD = 6,
  GO_TO_REF = 7,
};
ActionState actionState = IDLE_STATE;
ActionState prevActionState = IDLE_STATE;


class Quadrant {
  float calibration_m;
  float calibration_b;
  int num;
  int sensor_pin;
  int valve_pin;
  public:
    bool is_refilling;
    bool is_deflating;
    float ideal_pressure;
    float minuteSum;
  Quadrant(int n, int sp, int vp, float m, float b, float ip) {
    num = n;
    sensor_pin = sp;
    valve_pin = vp;
    calibration_m = m;
    calibration_b = b;
    is_refilling = false;
    is_deflating = false;
    ideal_pressure = ip;
    minuteSum = 0;
  }
  ~Quadrant() {
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
  ccSerial.begin(9600);
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
  
  actionState = IDLE_STATE;

  q1 = new Quadrant(1, sensorPin1, VALVE1, 0.00792, -0.252, 0.54);
  q2 = new Quadrant(2, sensorPin2, VALVE2, 0.00789, -0.302, 0.59);
  q3 = new Quadrant(3, sensorPin3, VALVE3, 0.00792, -0.281, 0.70);
  q4 = new Quadrant(4, sensorPin4, VALVE4, 0.00789, -0.262, 0.20);
}

// FUNCTIONS FOR SET-MODE -----------------------------------------

void idleState() {
  Serial.print("Locking Quadrants");
  digitalWrite(VALVE1, LOW);
  digitalWrite(VALVE2, LOW);
  digitalWrite(VALVE3, LOW);
  digitalWrite(VALVE4, LOW);
  digitalWrite(VALVE_OUT, LOW); //just in case
  digitalWrite(PUMP, LOW); // just in case
  pump_on = false;
  exhaust_on = false;
}

void unlockQuads(){
  Serial.print("Unlocking Quadrants");
  digitalWrite(VALVE1, HIGH);
  digitalWrite(VALVE2, HIGH);
  digitalWrite(VALVE3, HIGH);
  digitalWrite(VALVE4, HIGH);
}

void deflateAllMode() {
  unlockQuads(); // can try without this too I just nervy
  Serial.print("Deflating All");
  digitalWrite(VALVE_OUT, HIGH);
  digitalWrite(PUMP, LOW);
  exhaust_on = true;
  pump_on = false;
  
}

void inflateAllMode() {
  unlockQuads(); // can try without this too I just nervy
  Serial.print("Inflating All");
  digitalWrite(VALVE_OUT, LOW);
  digitalWrite(PUMP, HIGH);
  pump_on = true;
  exhaust_on = false;
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
  actionState = UNLOCK_QUAD;
}

void refillMode() {
  Serial.println("refill Mode");
  pump_on = true;
  if(q1->is_refilling) {
    q1->refillRoutine();
  }
  if(q2->is_refilling) {
  q2->refillRoutine();
  }
  if(q3->is_refilling) {
    q3->refillRoutine();
  }
  if(q4->is_refilling) {
  q4->refillRoutine();
  }
} // end refill mode 


void deflateMode() {
  if(q1->is_deflating) {
    if(q1->readPSI() <= q1->ideal_pressure) {
      q1->is_deflating = false;
      q1->closeValve();
    }
  }
  if(q2->is_deflating) {
    if(q2->readPSI() <= q2->ideal_pressure) {
      q2->is_deflating = false;
      q2->closeValve();
    }
  }
  if(q3->is_deflating) {
    if(q3->readPSI() <= q3->ideal_pressure) {
      q3->is_deflating = false;
      q3->closeValve();

    }
  }
  if(q4->is_deflating) {
    if(q4->readPSI() <= q4->ideal_pressure) {
      q4->is_deflating = false;
      q4->closeValve();
    }
  }
}

void goToRef(){
  //AS AN ALTERNATIVE WE COULD ALSO USE THE FIRST MINUTE AVG
  // BUT THIS SHOULD BE GOOD ENOUGH
  float curr_p1 = q1->readPSI();
  float curr_p2 = q2->readPSI();
  float curr_p3 = q3->readPSI();
  float curr_p4 = q4->readPSI();
  Serial.println("\nCurrent Pressures:\n");
  Serial.println("Q1: " + String(curr_p1) + "\n");
  Serial.println("Q2: " + String(curr_p2) + "\n");
  Serial.println("Q3: " + String(curr_p3) + "\n");
  Serial.println("Q4: " + String(curr_p4) + "\n\n");
  Serial.print("Current Pressures");
  if (user_is_seated(curr_p1, curr_p2, curr_p3, curr_p4)) {
    pressureCheck(q1, curr_p1);
    pressureCheck(q2, curr_p2);
    pressureCheck(q3, curr_p3);
    pressureCheck(q4, curr_p4);
  } else {
    // send message to UI (flash red maybe that not seated)
    Serial.println("user must be seated in order to reset back to ideal inflation");
  }
}

void pressureCheck(class Quadrant* q, float curr_pressure) {
      if(is_pressure_unacceptable(curr_pressure, q->ideal_pressure)){
      Serial.println("unideal pressure");
      if (curr_pressure < q->ideal_pressure) {
        Serial.println("set to refill");
        q->is_refilling = true;
      } else {
        Serial.println("set to deflate");
        q->is_deflating = true;
        q->openValve();
      }
   }

} // end checkQuad



// Algorithm helper functions
bool is_pressure_unacceptable(float current_pressure, float ideal_pressure){
  return abs(current_pressure - ideal_pressure) > acceptable_threshold;
}

bool user_is_seated(float q1_curr, float q2_curr, float q3_curr, float q4_curr) {
  // Initialize a counter for the number of floats less than 0.1
  int count_less_than_seated = 0;
  
  // Check each float and increment the counter if it's less than 0.1
  if (q1_curr < 0.1) count_less_than_seated++;
  if (q2_curr < 0.1) count_less_than_seated++;
  if (q3_curr < 0.1) count_less_than_seated++;
  if (q4_curr < 0.1) count_less_than_seated++;

  // If three or more floats are less than 0.1, return false
  if (count_less_than_seated > 1) {
    Serial.println("user is detected to be not seated");
    return false;
  } else {
    Serial.println("user is detected to be seated");
    return true; 
  }
}

ActionState getStateFromSerial(String inputString, String substring) {
  int index = inputString.indexOf(substring);
  if (index != -1 && index + substring.length() < inputString.length()) {
    char nextChar = inputString.charAt(index + substring.length());
    return nextChar - '0';
  }
  return -1;
}

void loop() {
      
      lastRefreshTime = millis();
      String receivedState = "";
      while(ccSerial.available()) {
        receivedState += ccSerial.read();
        Serial.println(receivedState);
      }
      if(receivedState.indexOf("ACTION STATE->") != -1) {
        actionState = getStateFromSerial(receivedState, "ACTION STATE->");
      }

     if(q1->is_refilling || q2->is_refilling || q3->is_refilling || q4->is_refilling){
       Serial.println("action state set to inflate");
       actionState = INFLATE_QUAD;
      } else if(q1->is_deflating || q2->is_deflating || q3->is_deflating || q4->is_deflating){
        Serial.println("action state set to deflate");
        actionState = DEFLATE_QUAD;
      } else if (prevActionState == INFLATE_QUAD || prevActionState == DEFLATE_QUAD || prevActionState == GO_TO_REF) {
        Serial.println("action state set to idle mode");
        actionState = IDLE_STATE;
      }

       // only change behaviour if new actionState OR inflate or deflate quad modes which are activated by the "go_to_ref" and loop 
       // until all quadrants are acceptable
       if (actionState == INFLATE_QUAD || actionState == DEFLATE_QUAD || actionState != prevActionState) {
          switch (actionState) {
          case SET_NEW_REF:
            recordAndSetIdealPressures();
            break;
          case INFLATE_ALL:
            inflateAllMode(); 
            break;
          case DEFLATE_ALL:
            deflateAllMode();
            break;  
          case INFLATE_QUAD:
            refillMode();
            break;
          case DEFLATE_QUAD:
            deflateMode();
            break;
          case GO_TO_REF:
            goToRef();
            break;
          case UNLOCK_QUAD:
             if(pump_on) {
                  digitalWrite(PUMP, LOW);
                  pump_on = false;
              }
              if (exhaust_on) {
                digitalWrite(VALVE_OUT, LOW);
                exhaust_on = false;
              }
            unlockQuads();
          case IDLE_STATE: 
              if(pump_on) {
                  digitalWrite(PUMP, LOW);
                  pump_on = false;
              }
              if (exhaust_on) {
                digitalWrite(VALVE_OUT, LOW);
                exhaust_on = false;
              }
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
              break;
        } // end switch action state 
       }
        prevActionState = actionState;

}// end loop 

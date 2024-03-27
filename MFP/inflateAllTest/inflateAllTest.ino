const int VALVE1 = 2;
const int VALVE2 = 3;
const int VALVE3 = 4;
const int VALVE4 = 5;
const int VALVE_OUT = 6;
const int PUMP = 7;
bool onceDone = false;


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
  delay(5000);
  if(!onceDone) {
    inflateAll();
    onceDone = true;
  }
}

void inflateAll() {
  Serial.print("here");
  digitalWrite(VALVE1, HIGH);
  digitalWrite(VALVE2, LOW);
  digitalWrite(VALVE3, LOW);
  digitalWrite(VALVE4, LOW);
  digitalWrite(VALVE_OUT, LOW);
  digitalWrite(PUMP, HIGH);
  delay(120000);
}

const int voltageSensor = A0;
const float vPow = 12.0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  // v = (analogRead(A0) * vPow) / 1024.0
  // Serial.print("Voltage = ");
  // Serial.println(v);
  
  // delay(1000);
  Serial.println("hello");
  delay(1000);
}

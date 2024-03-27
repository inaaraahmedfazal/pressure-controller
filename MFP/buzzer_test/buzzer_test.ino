const int buzzer = 26;

void setup() {
  // put your setup code here, to run once:
  pinMode(buzzer, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  tone(buzzer, 440);
  delay(500);
  noTone(buzzer);
  delay(500);
}

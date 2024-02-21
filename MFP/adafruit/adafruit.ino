int red = A2;
int green = A1;
int blue = A0;

void setup() {
  // put your setup code here, to run once:
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  analogWrite(red, 255);
  analogWrite(green, 70);
  analogWrite(blue, 0);
}

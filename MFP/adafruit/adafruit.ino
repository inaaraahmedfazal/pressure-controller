int red1 = A5;
int green1 = A4;
int blue1 = A3;

int red2 = MISO;
int green2 = MOSI;
int blue2 = SCK;

int buzzer = 0;
int vibrate = 1;

bool once = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(red1, OUTPUT);
  pinMode(green1, OUTPUT);
  pinMode(blue1, OUTPUT);

  pinMode(red2, OUTPUT);
  pinMode(green2, OUTPUT);
  pinMode(blue2, OUTPUT);
  
  pinMode(buzzer, OUTPUT);
  pinMode(vibrate, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  analogWrite(red1, 255);
  analogWrite(green1, 70);
  analogWrite(blue1, 0);

  analogWrite(red2, 0);
  analogWrite(green2, 0);
  analogWrite(blue2, 0);

  digitalWrite(buzzer, LOW);
  digitalWrite(vibrate, LOW);

  delay(1000);

  analogWrite(red1, 0);
  analogWrite(green1, 0);
  analogWrite(blue1, 0);

  analogWrite(red2, 255);
  analogWrite(green2, 70);
  analogWrite(blue2, 0);

  //digitalWrite(buzzer, HIGH);
  digitalWrite(vibrate, HIGH);

  delay(1000);

  if(!once) {

    once = true;
  }

}

int trigPin = 2;
int echoPin = 3;

void setup() {
  Serial.begin(9600);
  Serial.println("System Starting");
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  int duration = pulseIn(echoPin, HIGH);
  unsigned long distance = duration * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  delay(1000);
}
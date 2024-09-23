int in1 = 13;
int in2 = 12;
const int enA = 25;
const int rx = 32;
const int tx = 33;

const int channel = 0;
const int freq = 5000;
const int res = 8;

void setup() {
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(rx, INPUT);
  pinMode(tx, OUTPUT);
  digitalWrite(tx, HIGH);

  ledcSetup(channel, freq, res);
  ledcAttachPin(enA, channel);
}

void loop() {
  int signal = irsignal();

  if (signal == LOW) {
    ledcWrite(channel, 255);
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    tone(in1, 600);
    delay(300);
    tone(in1, 700);
    delay(200);
  } else {
    ledcWrite(channel, 0);
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    noTone(in1);
  }
}

int irsignal() {
  int signal = digitalRead(rx);
  return signal;
}

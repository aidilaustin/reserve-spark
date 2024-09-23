#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress mqtt_server(3, 0, 243, 196);
long lastMsg = 0;

char* parkingID = "0301";
const char* statusTopic = "spark/update-status/0301";
const char* updateSparkTopic = "update-spark-gate/0301";

EthernetClient ethClient;
PubSubClient client(ethClient);

int irSensor = A0;
int vibSensor = A1;
int in1 = 10;
int in2 = 9;
int in3 = 8;
int in4 = 7;
const int enA = 11;
const int enB = 6;

bool currentParkingStatus = false;
bool irDetection = false;
bool vibrationDetection = true;

unsigned long startTime;
unsigned long openGateStartTime;
unsigned long closeGateStartTime;
unsigned long vibDetectionTime;

int carPresenceThreshold = 30;
int alarmActive = 310;

void reconnect()
{
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("01")) {
      Serial.println("connected");
      client.subscribe(updateSparkTopic);
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (strcmp(topic, updateSparkTopic) == 0) {
    Serial.println("Get status");
    Serial.print("Message:");
    Serial.println(messageTemp);

    if (messageTemp == "open") {
      openGate();
      openGateStartTime = millis();
    } 
    else if (messageTemp == "close") {
      int distance = calculateDistance();
      if (distance > carPresenceThreshold) {
      closeGateStartTime = millis();
      closeGate();
      client.publish(statusTopic, "close");
    }
   }
 }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("System Starting");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
  } else {
    Serial.println("Success configured Ethernet Shield");
  }
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(irSensor, INPUT);
  pinMode(vibSensor, INPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
}

void loop() 
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    int distance = calculateDistance();
    int vibration = calculateVibration();

    if (vibrationDetection && vibration > alarmActive)
    {
    Serial.println("Vibration detected");
    startTime = millis();
    playAlarm();
    vibDetectionTime = millis();
      while (millis() - vibDetectionTime < 2000) {
      closeGateStartTime;
      }
    }

    if (distance < carPresenceThreshold && !currentParkingStatus)
    {
      currentParkingStatus = true;
      Serial.println("Car is parked");
    }
    else if (distance > carPresenceThreshold && currentParkingStatus)
    {
      currentParkingStatus = false;
      Serial.println("Car is not parked");
      Serial.println("Closing gate automatically...");
      client.publish(statusTopic, "close");
      closeGate();
      Serial.print("Topic: ");
      Serial.println(statusTopic);
      Serial.println("Gate is closed");
    }
  }
}

int calculateDistance()
{
  int distance = analogRead(irSensor);
  distance = map(distance, 0, 1023, 0, 100);
  return distance;
}

int calculateVibration()
{
  int vibrationValue = analogRead(vibSensor);
  //Serial.print("Vibration value: ");
  //Serial.println(vibrationValue);
  return vibrationValue;
}

void openGate()
{
  Serial.println("Open gate via Apps");
  if (!irDetection) {
  analogWrite(enA, 80); 
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  delay(400);
  stop();
  //delay(400);
  //analogWrite(enA, 100);
  //digitalWrite(in1, LOW);
  //digitalWrite(in2, HIGH);
  //delay(200);
  stop();
  startTime = millis();
  playTone();
    while (millis() - openGateStartTime < 2000) {
    openGateStartTime;
    }
  irDetection = true;
  vibrationDetection = false;
  }
  Serial.println("Open gate completed");
}

void closeGate()
{
  if (irDetection) {
  while (millis() - closeGateStartTime < 100) {
  closeGateStartTime;
  }
  analogWrite(enA, 150);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  delay(1800);
  stop();
  startTime = millis();
  playTone();
  irDetection = false;
  vibDetectionTime = millis();
  vibrationDetection = true;
  } 
    while (millis() - vibDetectionTime < 5000) {
    closeGateStartTime;
    }
  Serial.println("Close gate completed");
}

void stop() 
{
  analogWrite(enA, 0); 
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
}

void playTone()
{
  analogWrite(enB, 255); 
  digitalWrite(in4, HIGH);
  digitalWrite(in3, LOW);
  while (millis() - startTime < 500) {
    tone(in4, 600);
    delay(300);
    tone(in4, 700);
    delay(200);
  }
  analogWrite(enB, 0); 
  digitalWrite(in4, LOW);
  digitalWrite(in3, LOW);
  noTone(in4);
}

void playAlarm()
{
  analogWrite(enA, 150);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  delay(1800);
  stop();
  analogWrite(enB, 255); 
  digitalWrite(in4, HIGH);
  digitalWrite(in3, LOW);
  while (millis() - startTime < 10000) {
    tone(in4, 500);
    delay(200);
    tone(in4, 700);
    delay(150);
  }
  analogWrite(enB, 0); 
  digitalWrite(in4, LOW);
  digitalWrite(in3, LOW);
  noTone(in4);
}
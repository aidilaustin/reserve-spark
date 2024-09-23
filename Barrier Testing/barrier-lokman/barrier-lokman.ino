#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress mqtt_server(18, 142, 243, 160);
long lastMsg = 0;

char* parkingID = "0301";
const char* statusTopic = "spark/update-status/0301";
const char* updateSparkTopic = "update-spark-gate/0301";

EthernetClient ethClient;
PubSubClient client(ethClient);

int trigPin = 2;
int echoPin = 3;
int in1 = 8;
int in2 = 9;
const int enA = 10;

bool currentParkingStatus = false;
bool sensorDetection = false;

unsigned long openGateStartTime;
unsigned long closeGateStartTime;

int carPresenceThreshold = 30;

void reconnect()
{
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("Lokman2")) { //Change as needed
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
      unsigned long distance = calculateDistance();
      if (distance > carPresenceThreshold) {
      closeGateStartTime = millis();
      closeGate();
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

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enA, OUTPUT);
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

    unsigned long distance = calculateDistance();

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
      closeGate();
      Serial.print("Topic: ");
      Serial.println(statusTopic);
      Serial.println("Gate is closed");
    }
  }
}

unsigned long calculateDistance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  int duration = pulseIn(echoPin, HIGH);
  unsigned long distance = duration * 0.034 / 2;
  return distance;
}

void openGate()
{
  Serial.println("Open gate via Apps");
  if (!sensorDetection) {
  analogWrite(enA, 60); 
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  delay(600);
  stop();
    while (millis() - openGateStartTime < 2000) {
    openGateStartTime;
    }
  sensorDetection = true;
  }
  Serial.println("Open gate completed");
}

void closeGate()
{
  if (sensorDetection) {
  while (millis() - closeGateStartTime < 100) {
  closeGateStartTime;
  }
  analogWrite(enA, 130);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  delay(1400);
  stop();
  sensorDetection = false;
  Serial.println("Close gate completed");
  }
}

void stop() 
{
  analogWrite(enA, 0); 
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
}
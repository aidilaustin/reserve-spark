#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "rmsb7577@unifi";
const char* password = "0192225891abc";
const char* mqtt_server = "3.0.243.198";
long lastMsg = 0;

char* parkingID = "0302";
const char* statusTopic = "spark/update-status/0302";
const char* updateSparkTopic = "update-spark-gate/0302";

WiFiClient espClient;
PubSubClient client(espClient);

int in1 = 13;
int in2 = 12;
const int enA = 25;
const int rx = 32; 
const int tx = 33; 
int irSensor = 34;

const int channel = 0;
const int freq = 5000; 
const int resolution = 8; 

unsigned long startTime;
unsigned long openGateStartTime;
unsigned long closeGateStartTime;

bool currentParkingStatus = false;
bool irDetection = false;

int carPresenceThreshold = 30;

void reconnect()
{
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("02")) {
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

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enA, OUTPUT);
  pinMode(irSensor, INPUT);
  pinMode(rx, INPUT);
  pinMode(tx, OUTPUT);
  digitalWrite(tx, HIGH);

  ledcSetup(channel, freq, resolution);
  ledcAttachPin(enA, channel);
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

    if (distance < carPresenceThreshold && !currentParkingStatus)
    {
      currentParkingStatus = true;
      client.publish(statusTopic, "open");
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
  }
  else if (signal == HIGH) {
    ledcWrite(channel, 0); 
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    noTone(in1);
  }
}

int calculateDistance()
{
  int distance = analogRead(irSensor);
  distance = map(distance, 0, 1023, 0, 100);
  return distance;
}

int irsignal() {
  int signal = digitalRead(rx);
  return signal;
}

void openGate()
{
  Serial.println("Open gate via Apps");
  if (!irDetection) {
  analogWrite(enA, 80); 
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  delay(500);
  stop();
  startTime = millis();
    while (millis() - openGateStartTime < 1000) {
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
  analogWrite(enA, 160);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  delay(1800);
  stop();
  startTime = millis();
  irDetection = false;
  vibDetectionTime = millis();
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
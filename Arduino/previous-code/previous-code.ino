#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Direccion MAC
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// IP address
IPAddress mqtt_server(18, 142, 243, 160); // http://18.142.243.160/
long lastMsg = 0;


// Topic
char* parkingID = "0301";
const char* statusTopic = "spark/update-status/0301";
const char* updateSparkTopic = "update-spark-gate/0301";

EthernetClient ethClient;
PubSubClient client(ethClient);

int trigPin = 2;
int echoPin = 3;

// motor pin
int in1 = 8;
int in2 = 9;

bool currentParkingStatus = false;

int carPresenceThreshold = 10;

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("arduinoClient1")) {
      Serial.println("connected");
      client.subscribe(updateSparkTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
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

    } else if (messageTemp == "close") {
      int distance = calculateDistance();
      if (distance < carPresenceThreshold)
      {
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
      Serial.println("Car is parked");
    }
    else if (distance > carPresenceThreshold && currentParkingStatus)
    {
      currentParkingStatus = false;
      Serial.println("Car is not parked");
      Serial.println("Closing Gate automatically................");
      client.publish(statusTopic, "close");
      Serial.print("Topic: ");
      Serial.println(statusTopic);
      delay(1000);
      Serial.println("Gate is closed");
    }


  }
}

int calculateDistance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  int duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;

  return distance;
}

void openGate()
{
  Serial.println("Open Gate from the apps.....");
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  delay(5000);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  Serial.println("Open gate completed");
}

void closeGate()
{
  Serial.println("Close Gate from the apps.....");
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  delay(5000);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  Serial.println("Close gate completed");
}

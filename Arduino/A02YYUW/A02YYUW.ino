#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX 
connect Yellow wire to D3 and White wire to D2

unsigned char data[4]={};
float distance;

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600); 
  Serial.println("System Starting");
}

void loop()
{
    do {
        for(int i=0; i<4; i++) {
            data[i] = mySerial.read();
        }
    } while (mySerial.read() == 0xff);

    mySerial.flush();

    if (data[0] == 0xff) {
        int sum;
        sum = (data[0] + data[1] + data[2]) & 0x00FF;
        if (sum == data[3]) {
            distance = (data[1] << 8) + data[2];
            distance = distance / 10;
            Serial.println(distance);
        }
    }
    delay(100);
}

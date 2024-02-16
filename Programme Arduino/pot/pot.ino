
#include <Servo.h>

Servo servo;  // create servo object to control a servo

int potpin = A0; 

void setup() {
  servo.attach(9);  
  Serial.begin(9600);
}

void loop() {
 
  int sensorValue = analogRead(A0);
  Serial.print(sensorValue);
  sensorValue = map(sensorValue, 0, 1023, 0, 180);     // scale it for use with the servo (value between 0 and 180)
  servo.write(sensorValue);                  // sets the servo position according to the scaled value
  Serial.println(sensorValue);
  delay(100);
}

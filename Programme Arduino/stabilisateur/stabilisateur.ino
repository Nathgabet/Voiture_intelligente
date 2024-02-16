#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "ServoTimer2.h"
#define CS 10
Adafruit_MPU6050 mpu;


ServoTimer2 servo1;
int stab = 0;

void setup() {

  servo1.attach(4);
  Serial.begin(115200);

  Serial.println("Adafruit MPU6050 test!");
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  Serial.println("+-8G");
 
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  Serial.println("+- 500 deg/s");
  
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  Serial.println("21 Hz");
   
  Serial.println("");
  delay(100);

}

void loop() {
  

  while(1){
  int stab = 1500;
  int z=0;

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
 
  z = g.gyro.z;
  Serial.print("Z: ");
  Serial.println(z);
  stab = 1500 +(93* -z );
  
  if(stab > 2250)
    stab = 2250;
  else if (stab <1500)
    stab = 750;
  
  servo1.write(stab);
  delay(100);
  }
  
}
/*
void stabilisateur(){

  int stab = 1500;

  int z=0;

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
 
  z = g.gyro.z;
  Serial.print("Z: ");
  Serial.println(z);
  stab = 1500 +(93* -z );
  
  if(stab > 2250)
    stab = 2250;
  else if (stab <1500)
    stab = 750;

}*/
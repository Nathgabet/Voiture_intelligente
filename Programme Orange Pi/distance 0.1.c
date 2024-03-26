#include <stdio.h>
#include <wiringPi.h>

#define TRIG_PIN 18
#define ECHO_PIN 24

int main() {
    // Initialize wiringPi library
    wiringPiSetup();

    // Set TRIG_PIN as output and ECHO_PIN as input
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // Variables to store the start and end time of the pulse
    double startTime, endTime;

    // Variable to store the distance
    double distance;

    // Loop forever
    while(1) {
        // Send a 10us pulse on TRIG_PIN to trigger the sensor
        digitalWrite(TRIG_PIN, 1);
        delayMicroseconds(10);
        digitalWrite(TRIG_PIN, 0);

        // Wait for the pulse to be received on ECHO_PIN
        while(digitalRead(ECHO_PIN) == 0);
        startTime = micros();
        while(digitalRead(ECHO_PIN) == 1);
        endTime = micros();

        // Calculate the distance
        distance = (endTime - startTime) / 58.0;

        // Print the distance
        printf("Distance: %f cm\n", distance);
    }

    return 0;
}


struct timespec start, stop;



  while (digitalRead(ECHO_PIN) == 0);
        clock_gettime(CLOCK_MONOTONIC, &start);
        while (digitalRead(ECHO_PIN) == 1);
        clock_gettime(CLOCK_MONOTONIC, &stop);

        // Calculate the time taken for the pulse to travel to the object and back
        long travelTime = (stop.tv_sec - start.tv_sec) * 1000000000 + (stop.tv_nsec - start.tv_nsec);



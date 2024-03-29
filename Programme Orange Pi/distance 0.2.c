
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wiringPi.h>

#define TRIG 23
#define ECHO 25
#define LED  22

long distance_raw(int pin){

        struct timespec start, stop;

        while(digitalRead(ECHO) == LOW){
               clock_gettime(CLOCK_MONOTONIC, &start);
        }
        while(digitalRead(ECHO) == HIGH){
                clock_gettime(CLOCK_MONOTONIC, &stop);
        }

       return ((stop.tv_sec - start.tv_sec)*100000000 + (stop.tv_nsec - start.tv_nsec)) /58.0;

}

int main() {

        wiringPiSetup();

        pinMode(TRIG, OUTPUT);
        pinMode(ECHO, INPUT);
        pinMode(LED, OUTPUT);
        long distance;
        double longueur;

        while(1) {

        digitalWrite(TRIG, HIGH);
        delayMicroseconds(5);
        digitalWrite(TRIG, LOW);

        distance = distance_raw(ECHO);

        if(distance> 1000 && distance<10000){
                longueur = distance_raw(ECHO) /58.0;

                printf("Distance: %f cm\n", longueur);
                delay (100);
        }
    }

    return 0;
}

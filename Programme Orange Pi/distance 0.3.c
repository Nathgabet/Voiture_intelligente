/*****************************************************************************

Programme permettant de meusurer la distance avec le capteur

*****************************************************************************

Raccordement du capteur :

Vcc  -> +5V
Trig -> Pin 23
Echo -> Pin 25
Gnd  -> Gnd

*****************************************************************************

Compilation du programme :

gcc -Wall distance.c -lwiringPi -o distance

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <wiringPi.h>

#define TRIG 23
#define ECHO 25
#define LED  22

long distance_raw(int pin){

        struct timeval start, stop;

        while(digitalRead(ECHO) == LOW){
               gettimeofday(&start, NULL);
        }
        while(digitalRead(ECHO) == HIGH){
                gettimeofday(&stop, NULL);
        }

       return ((stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec)*0.017);

}

int main() {

        printf("Initialisation du programme \n");

        wiringPiSetup();

        pinMode(TRIG, OUTPUT);
        pinMode(ECHO, INPUT);
        pinMode(LED, OUTPUT);

        double longueur = 0;
        int meusureF = 0,  meusureJ = 0;

        printf("Lancement du programme du programme \n");

        while(meusureJ <100) {

        digitalWrite(TRIG, LOW);
        delayMicroseconds(5);
        digitalWrite(TRIG, HIGH);
        delayMicroseconds(5);
        digitalWrite(TRIG, LOW);

        longueur = distance_raw(ECHO);

        if(longueur > 3 && longueur < 100){

                printf("Distance: %f cm\n", longueur);
                meusureJ ++;
                delay (100);
        }
        else{
                meusureF++;
        }

    }
        printf("Nb meusures juste: %d \nNb meusues fausses :%d\n", meusureJ, meusureF);
    return 0;
}
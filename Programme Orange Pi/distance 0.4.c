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
#include <unistd.h>
#include <fcntl.h> // pour open
#include <sys/ioctl.h> // pour ioctl
#include <linux/i2c-dev.h>

#define TRIG 23
#define ECHO 25
#define LED  26


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

        piThreadCreate (flag) ;

        double longueur = 0;
        int mesure , mesureF = 0;

        printf("Lancement du programme du programme \n");

        while(1) {
                for (mesure = 1; mesure <= 100; mesure ++){
                        digitalWrite(TRIG, LOW);
                        delayMicroseconds(5);
                        digitalWrite(TRIG, HIGH);
                        delayMicroseconds(5);
                        digitalWrite(TRIG, LOW);

                        longueur = distance_raw(ECHO);

                        if(longueur > 3 && longueur < 100){

                                printf("Distance: %f cm\n", longueur);
                                delay (50);
                        }
                        else{
                                mesureF++;
                        }

                }
                printf("Fiabilite: %d/100 \n", mesure-mesureF);
                sleep(1);
                mesureF = 0;
        }

        return 0;
}

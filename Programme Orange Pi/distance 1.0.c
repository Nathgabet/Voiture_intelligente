/*****************************************************************************

Programme permettant de meusurer la distance avec le capteur

*****************************************************************************

Raccordement du capteur :

Vcc  -> +5V
Trig -> Pin 23/258
Echo -> Pin 25/272
Gnd  -> Gnd

*****************************************************************************

Compilation du programme :

gcc -Wall distance.c lxlib.c -lpthread -o distance

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>
#include <linux/gpio.h>
#include <unistd.h>
#include <fcntl.h> // pour open
#include <sys/ioctl.h> // pour ioctl
#include <string.h>
#include "lxlib.h"
#include <pthread.h>

#define TRIG 258
#define ECHO 272
#define LED  259
#define BOUCLE 260

long distance_raw(struct pin *echo);

void *arret( void *arg){

        int *boucle = (int *)arg;
        struct pin arret;
        int value;

        arret.fd = 3;
        arret.gpio = BOUCLE;
        if(GpioIn(&arret)<0){
                pthread_exit(arg);
        }

        for (;;){
                value = GpioRead(&arret) ;  // On
                msleep (10) ;               // mS
                if (value == 1){
                        *boucle = 0;
                        pthread_exit(NULL);
                }
                else{
                        *boucle = 1;
                }
        }
}

int main() {

        printf("Lanchement du programme \n");

        struct pin echo, led, trig;
        int fd, boucle =1;
        double longueur = 0;
        int mesure , mesureF = 0;
        pthread_t t1;

        if( (fd = InitGpio())<0){
                perror("Erreur : Initialisation Gpio");
                return -1;
        }

        trig.fd = fd;
        trig.gpio = TRIG;
        if(GpioOut(&trig)<0){
                close(fd);
                return -1;
        }

        echo.fd = fd;
        echo.gpio = ECHO;
        if(GpioIn(&echo)<0){
                close(fd);
                return -1;
        }

        led.fd = fd;
        led.gpio = LED;
        if(GpioOut(&led)<0){
                close(fd);
                return -1;
        }

        if(pthread_create(&t1, NULL, *arret, &boucle)){
                perror("Error: Creation du thread");
                return -1;
        }

        printf("Initialisation fini, lancement du programme\n");

        while(boucle) {

                mesure =1;

                while (mesure <= 100 && boucle){
                        GpioWrite(&trig, LOW);
                        usleep(5);
                        GpioWrite(&trig, HIGH);
                        usleep(5);
                        GpioWrite(&trig, LOW);

                        longueur = distance_raw(&echo);

                        if(longueur > 3 && longueur < 100){

                                printf("Distance: %f cm\n", longueur);
                                msleep (50);
                        }
                        else{
                                mesureF++;
                        }
                        mesure++;

                }
                printf("Fiabilite: %d/100 \n", mesure-mesureF);
                sleep(1);
                mesureF = 0;
        }

        printf("Arret du programme\n");
        pthread_cancel(t1);
        if(GpioWrite(&led,0)<0){
                close(fd);
                return -1;
        }
	close (fd);
        return 0;
}

long distance_raw(struct pin *echo){

        struct timeval start, stop;

        while(GpioRead(echo) == LOW){
               gettimeofday(&start, NULL);
        }
        while(GpioRead(echo) == HIGH){
                gettimeofday(&stop, NULL);
        }

       return ((stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec)*0.017);

}
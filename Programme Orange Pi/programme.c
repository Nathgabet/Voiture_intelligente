#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <pthread.h>
#include <linux/joystick.h>
#include <linux/gpio.h>
#include <string.h>
#include "lxlib.h"

#define TRIG 272
#define ECHO 258

struct boucle{

	int principale;
	int secondaire;

};

long distance_raw(struct pin *echo);

void *arret( void *arg){

	const char *device;
    	int js;
    	struct boucle *croix= (struct boucle *) arg;
    	struct js_event event;

        device = "/dev/input/js0";
	js = open(device, O_RDONLY);
    	if ( js <0){
        	perror("Could not open joystick");
	}

    	while (read_event(js, &event) == 0){

		switch (event.type){
	        	case JS_EVENT_BUTTON:
				if(event.type == JS_EVENT_BUTTON && event.number ==1 && event.value ==1 && (*croix).secondaire == 1){
					(*croix).secondaire = 0;
				}
				else if(event.type == JS_EVENT_BUTTON && event.number ==1 && event.value ==1){
					(*croix).principale = 0;
				}

				break;
                	default:
                		break;
        	}
        	fflush(stdout);
    	}
	close(js);
}

int main (int argc, char *argv[]) {

	struct pin echo, trig;
	int fd_gpio, mesure, mesureF =0;;
	double longueur =0;

	struct boucle exit;
	pthread_t t1;

	if( (fd_gpio = InitGpio()) < 0){
		perror("Erreur : Initialisation Gpio");
                return -1;
	}

	trig.fd = fd_gpio;
	trig.gpio = TRIG;
	if(GpioOut(&trig) < 0){
                close(fd_gpio);
                return -1;
        }

	echo.fd = fd_gpio;
	echo.gpio = ECHO;
	if(GpioIn(&echo)<0){
                close(fd_gpio);
                return -1;
        }

	exit.principale = 1;
	exit.secondaire = 1;

	if(pthread_create(&t1, NULL, *arret, &exit)){
        	perror("Error: Creation du thread");
        	return -1;
    	}

	while (exit.principale ){

		printf("Lancement du programme de distance \n");

		while (exit.secondaire){

			mesure =1;

			while (mesure <= 100 ){
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
		printf("Arret du programme \n");
		msleep(100);

	}

	printf("\nfin du programme \n");
	pthread_cancel(t1);
	close(fd_gpio);

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

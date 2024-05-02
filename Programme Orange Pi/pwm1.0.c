/*
*****************************************************************************
Duty cycle:
	servomoteur
9.31% - 7.33% - 5.31

	moteur puissance max (-20% pmin)
5.00% - 7.50% - 10.00%

Temps à l'état haut en µseconde:
	servomoteur
1862 - 1466 - 1062

        moteur puissance max (-20% pmin)
1000 - 1500 - 2000

*****************************************************************************

gcc -Wall pwm.c lxlib.c -pthread -o pwm

*/

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <pthread.h>
#include "lxlib.h"

uint8_t stop = 1;

struct pwm{

	struct pin pin;
	pthread_t thread;
	int period;
	int duty_cycle;

};

void *SetPwm(void *arg){

	struct pwm *pwm = (struct pwm *)arg;
	struct pin gpio;
	int highsleep,lowsleep;

	gpio = (* pwm).pin;

	highsleep = (*pwm).duty_cycle;
	lowsleep = (*pwm).period - (*pwm).duty_cycle;


	if(GpioWrite(&gpio, 0)<0){
		pthread_exit(NULL);
	}
	printf("Début du signal\n");

	while(stop){

		GpioWrite( &gpio,1);
		usleep(highsleep);
		GpioWrite( &gpio,0);
                usleep(lowsleep);

	}

	pthread_exit(NULL);
}

int InitPwm(struct pwm *pwm1){

	pthread_t t1 = (*pwm1).thread ;
	(*pwm1).period = 19750;

	if(pthread_create(&t1, NULL, *SetPwm, pwm1)){
        	perror("Error: Creation du thread");
		return -1;
    	}
	printf("Thread create \n");

	return 1;
}

int  main (void){

	struct pin pwm;
	struct pwm pwm1;
	pthread_t t1;

	if((pwm.fd = InitGpio()) < 0){
		perror("Erreur : Initialisation Gpio");
		return -1;
	}

	pwm.gpio = PWM1;

	if(GpioOut(&pwm) < 0){
		close(pwm.fd);
                return -1;
        }

	pwm1.pin = pwm;
	pwm1.duty_cycle = 2000;

	if(InitPwm(&pwm1) < 0){
		close(pwm.fd);
                return -1;
        }
	t1 = pwm1.thread;
	sleep(10);
	stop = 0;
	msleep(500);
	pthread_cancel(t1);
	close(pwm.fd);

	return 0;
}

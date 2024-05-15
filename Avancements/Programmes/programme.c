/*

gcc -Wall programme.c lxlib.c -lpthread -o programme

ajouter watchdog sur la connection js

*/

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
#include <linux/i2c-dev.h>
#include <string.h>
#include "lxlib.h"

#define TRIG 272
#define ECHO 258

/**
 * struct boucle - permet de gérer le changement et l'arret du programme
 * @principal : est à 1 si toutes les variables sont initialisés 
 * @secondaire : est à 1 si l'on souhaite naviger parmis les séquences (menu)
 * @croix : bouton passant à 1 pour lancer la séquence choisi
 * @rond : bouton permettant d'arréter une séquence ou arrêter le programme
 * @triangle : variable d'itération permettant de changer de séquence
 * @event : variable passant à 1 lorsque l'on appui sur un bouton
*/
struct boucle{

	int principale;
	int secondaire;
	int croix;
	int rond;
	int triangle;
	int event;

};

/**
 * InitBoucle - Attribut des valeurs à chaques parametres permettant de 
 * lancer le programme principale dans un état d'attente
*/
void InitBoucle(struct boucle *init){

	(*init).principale = 1;
	(*init).secondaire = 0;
	(*init).croix = 0;
	(*init).rond = 0;
	(*init).triangle = 0;

}

/**
 * *arret - permet de gérer le fonctionnement des boucles du main 
 * grâce une la manette 
*/
void *arret( void *arg){

    	int js;
    	struct boucle *boucle= (struct boucle *) arg;
    	struct js_event event;

// Initialisation du port du joystick
		js = open("/dev/input/js0", O_RDONLY);
    	if ( js <0){
        	perror("Ne peut pas ouvrir le port joystick");
		(*boucle).principale =0;
	}
	else{
		printf("Port Joystick ouvert\n");
	}

// Lecture et gestion des boutons
    	while ((read_event(js, &event) == 0) && ((*boucle).principale ==1)){

			if (event.type == JS_EVENT_BUTTON){
				(*boucle).event = 1;
	        	switch (event.number){

					case 0:
						if (((*boucle).secondaire == 0) && (event.value ==0)){
							(*boucle).secondaire = 1;
						}
						break;

					case 1 :
						if( (event.value ==0) && ((*boucle).secondaire == 1) ){
							(*boucle).secondaire = 0;
						}
						else if ((*boucle).secondaire == 0){
							(*boucle).principale = 0;
						}
						break;

					case 2:
						if ( (event.value == 0)&& ((*boucle).secondaire== 0) ){
                	    	(*boucle).triangle++ ;
                	    }
						break;

					case 3:
						if ( (event.value ==0)&& ((*boucle).secondaire == 0) ){
        	                (*boucle).triangle = 0;
            	       	}
						break;

					default:
    	            	break;
				}
	        	fflush(stdout);

			}
		(*boucle).event = 0;
        
		}

// Sortie et arret du thread
	close(js);
	pthread_exit(NULL);
}

int main (int argc, char *argv[]) {

	struct boucle exit;
	pthread_t t1, t2;

	struct pin echo, trig;
	int fd_gpio, mesure, mesureF =0;;
	double longueur =0;

	int fd_i2c;
	float Dmax = 0, Gmax =0;
	int16_t accel[3], gyro, temp;

	struct pin pinpwm;
	pwm pwm1;

	float f_gyro;

	InitBoucle(&exit);

//Initialisation du port GPIO
	if( (fd_gpio = InitGpio()) < 0){
        return -1;
	}

// Paramettrage des pins pour le capteur de distance
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
// Initialisation du port i2c
	if((fd_i2c = Initi2c()) < 0){
		close(fd_gpio);
		return -1;
	}

	exit.principale = 1;
	exit.secondaire = 0;

//Initialisation du thread d'arret
	if(pthread_create(&t1, NULL, *arret, &exit)){
        	perror("Error: Creation du thread");
        	close(fd_gpio);
		close(fd_i2c);
		return -1;
    	}
	printf("Thread d'arret créér\n");

	msleep(500);
	if(exit.principale ==0){
		pthread_cancel(t1);
		close(fd_gpio);
                close(fd_i2c);
                return -1;
	}

// Initialisation et paramettrage du gyroscope
	if(ioctl(fd_i2c, I2C_SLAVE, 0x68) < 0){
                perror("Error to setting slave adress");
                pthread_cancel(t1);
                close(fd_gpio);
                close(fd_i2c);
                return -1;
        }

        if(verif(&fd_i2c)<0){
                pthread_cancel(t1);
                close(fd_gpio);
                close(fd_i2c);
                return -1;
        }

        if(reset(&fd_i2c)!=0){
                perror("Error réinitialisation");
                pthread_cancel(t1);
                close(fd_gpio);
                close(fd_i2c);
                return -1;
        }

        if(config(&fd_i2c)!=0){
                perror("Error configuration");
                pthread_cancel(t1);
                close(fd_gpio);
                close(fd_i2c);
                return -1;
        }

        if(verif(&fd_i2c)<0){
                pthread_cancel(t1);
                close(fd_gpio);
                close(fd_i2c);
                return -1;
        }

	pinpwm.fd = fd_gpio;
	pinpwm.gpio = PWM1;

	if(GpioOut(&pinpwm) < 0){
          	perror("setting pin du Pwm");
		pthread_cancel(t1);
                close(fd_gpio);
                close(fd_i2c);
		return -1;
        }
	pwm1.pin = pinpwm;
	pwm1.duty_cycle = 1466;//pour desactiver la commande du servomoteur

	if(InitPwm(&pwm1) < 0){
		perror("Initilisation Pwm");
		pthread_cancel(t1);
                close(fd_gpio);
                close(fd_i2c);
                return -1;
        }

	t2 = pwm1.thread;
	printf("Initialisation Pwm fini\n");

	printf("Initialisation fini, lancement du programme\n");

// Boucles des différents programmes
	while (exit.principale ){
		if(exit.event){
			msleep(10);
			switch(exit.triangle){

				case 0:
					printf("Capteur de distance\n");
					break;
				case 1:
					printf("Gyroscope\n");
					break;
				case 2:
					printf("Stabilisateur\n");
					break;
				default:
					printf("Sectionner un programme\n");
					break;
			}
			msleep(100);
			}
			while (exit.secondaire){

				switch(exit.triangle){

					case 0:
						mesure = 0;

						while ( (mesure <= 100) && (exit.secondaire) ){
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
        						else if(longueur < 0){
                						printf("Perte de la mesure");
        						}
							else {
								mesureF++;
							}
							mesure++;

    						}
        					printf("Nb bonne valeur: %d/100 \n", mesure-mesureF);
						sleep(1);
						mesureF = 0;
						break;
					case 1:
						mpu_read_raw(&fd_i2c, accel, &gyro, &temp);

						if(((float)gyro)/65.5 < Dmax){
							Dmax = ((float)gyro)/65.5 ;
						}
						else if (((float)gyro)/65.5 > Gmax){
				                	Gmax = ((float)gyro)/65.5 ;
                				}

		                		printf(" accel X: %6.3f accel Y: %6.3f\n", ((float)accel[0])/16875.0,((float)accel[1]) / -16875.0);
                				printf(" gyro X: %6.3f\n",((float)gyro)/65.5);
                				printf(" temp: %5.2f\n", (((float)temp) / 340.0) + 36.53);
						printf(" Gmax = %f , Dmax = %f \n", Gmax, Dmax);
		                		msleep(100);
						break;
					case 2:
						mpu_read_raw(&fd_i2c, accel, &gyro, &temp);
               					printf(" gyro X: %6.3f\n",((float)gyro)/65.5);

						f_gyro = ((float)gyro)/65.5;
						if(f_gyro > 160){
							f_gyro = 130;
						}
						else if(f_gyro < -130){
							f_gyro = -130;
						}

						pwm1.duty_cycle = f_gyro*6.15+ 1466;
						printf(" Duty cycle %d \n", pwm1.duty_cycle);
						break;
					default:
						printf("Programme lancé: Sectionner un programme\n");
						break;
				}

				}



		}

		printf("\nFermeture du programme \n");
		msleep(500);
		pthread_cancel(t1);
		pthread_cancel(t2);
		close(fd_gpio);
		close(fd_i2c);
		msleep(500);
		return 0;

}



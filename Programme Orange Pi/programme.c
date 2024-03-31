/*

gcc -Wall programme.c lxlib.c -lpthread -o programme

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

struct boucle{

	int principale;
	int secondaire;
	int croix;
	int rond;
	int triangle;
	int carre;

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

    	while ((read_event(js, &event) == 0) && ((*croix).principale ==1)){

		switch (event.type){
	        	case JS_EVENT_BUTTON:
				if(event.number ==1 && event.value ==1 ){
					if((*croix).secondaire == 1){
						(*croix).secondaire = 0;
					}
					else{
						(*croix).principale = 0;
					}
					(*croix).rond = event.value  ;

				}
				else if (event.number == 0 && event.value ==1){
					if((*croix).secondaire == 0){
						(*croix).secondaire = 1;
					}
					(*croix).croix = event.value  ;

				}
				else if (event.number == 2 && event.value ==1){
                                        if((*croix).triangle == 1){
                                                (*croix).triangle = 0;
                                        }
					else{
                                        	(*croix).triangle = 1;
					}
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

	int fd_i2c;
	int16_t accel[3]= {10}, gyro, temp;

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

	fd_i2c = open("/dev/i2c-1", O_RDWR);

        if(fd_i2c < 0){
                perror("Error openning the i2c Bus");
                return -1;
        }
        printf("Bus Open \n");

	exit.principale = 1;
	exit.secondaire = 1;

	if(pthread_create(&t1, NULL, *arret, &exit)){
        	perror("Error: Creation du thread");
        	return -1;
    	}

	if(ioctl(fd_i2c, I2C_SLAVE, 0x68) < 0){
                perror("Error to setting slave adress");
                close(fd_i2c);
                return -1;
        }

        if(verif(&fd_i2c)<0){
                close(fd_i2c);
                return -1;
        }

        if(reset(&fd_i2c)!=0){
                perror("Error réinitialisation");
                close(fd_i2c);
                return -1;
        }

        if(config(&fd_i2c)!=0){
                perror("Error configuration");
                close(fd_i2c);
                return -1;
        }

        if(verif(&fd_i2c)<0){
                close(fd_i2c);
                return -1;
        }
	printf("Initialisation fini, lancement du programme\n");

	while (exit.principale ){

		printf("Lancement du programme de distance \n");

		while (exit.secondaire && exit.triangle ==1){

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
        			else{
                			mesureF++;
        			}
				mesure++;

    			}
        		printf("Fiabilite: %d/100 \n", mesure-mesureF);
			sleep(1);
			mesureF = 0;

		}
		printf("Programme en pause \n");
		msleep(100);

		while(exit.secondaire && exit.triangle == 0 ){

			if(mpu_read_raw(&fd_i2c, accel, &gyro, &temp)<0){
                        	perror("Error reading data");
                        	close(fd_i2c);
                        	return -1;
                	}

                	printf(" accel X: %6.3f accel Z: %6.3f\n", ((float)accel[0])/16875.0,((float)accel[2])/17940.48);
                	printf(" gyro Z: %6.3f\n",((float)gyro)/65.5);
                	printf(" temp: %5.2f\n", (((float)temp) / 340.0) + 36.53);

                	msleep(100);

		}



	}

	printf("\n Fermeture du programme \n");
	sleep(1);
	pthread_cancel(t1);
	close(fd_gpio);
	close(fd_i2c);

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

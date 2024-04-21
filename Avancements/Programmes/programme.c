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

struct boucle{

	int principale;
	int secondaire;
	int croix;
	int rond;
	int triangle;
	int event;

};

void InitBoucle(struct boucle *init){

	(*init).principale = 1;
	(*init).secondaire = 0;
	(*init).croix = 0;
	(*init).rond = 0;
	(*init).triangle = 0;

}

long distance_raw(struct pin *echo);

void *arret( void *arg){

		const char *device;
    	int js;
    	struct boucle *boucle= (struct boucle *) arg;
    	struct js_event event;

        device = "/dev/input/js0";
		js = open(device, O_RDONLY);
    	if ( js <0){
        	perror("Could not open joystick");
		(*boucle).principale =0;
	}
	else{
		printf("Joystick connecté \n");
	}

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
					if ( (event.value == 0) ){
                    				(*boucle).triangle++ ;
                    			}

					break;
				case 3:
					if ( (event.value ==0) ){
                        			(*boucle).triangle = 0;
                   			}

				default:
                		break;

			}
	        fflush(stdout);
		}
		(*boucle).event = 0;
        }
 
	close(js);
	pthread_exit(NULL);
}

int main (int argc, char *argv[]) {

	struct pin echo, trig;
	int fd_gpio, mesure, mesureF =0;;
	double longueur =0;

	int fd_i2c;
	int16_t accel[3]= {10}, gyro, temp;

	struct boucle exit;
	pthread_t t1;

	InitBoucle(&exit);

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
		close(fd_gpio);
                return -1;
        }
        printf("Bus Open \n");

	exit.principale = 1;
	exit.secondaire = 0;

	if(pthread_create(&t1, NULL, *arret, &exit)){
        	perror("Error: Creation du thread");
        	close(fd_gpio);
		close(fd_i2c);
		return -1;
    	}
	msleep(500);
	if(exit.principale ==0){
		pthread_cancel(t1);
		close(fd_gpio);
                close(fd_i2c);
                return -1;
	}

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
	printf("Initialisation fini, lancement du programme\n");

	while (exit.principale ){
		if(exit.event){
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

			}
			msleep(10);
			printf("Croix: %d, Rond: %d, tri: %d, second: %d, prin: %d \n", exit.croix ,exit.rond ,exit.triangle ,exit.secondaire ,exit.principale );

			while(exit.secondaire && exit.triangle == 0 ){

				if(mpu_read_raw(&fd_i2c, accel, &gyro, &temp) < 0){
                    perror("Error reading data");
                    pthread_cancel(t1);
                close(fd_gpio);
                close(fd_i2c);

                    return -1;
                }

                printf(" accel X: %6.3f accel Y: %6.3f\n", ((float)accel[0])/16875.0,((float)accel[1]) / -16875.0);
                printf(" gyro X: %6.3f\n",((float)gyro)/65.5);
                printf(" temp: %5.2f\n", (((float)temp) / 340.0) + 36.53);

                msleep(100);

			}

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

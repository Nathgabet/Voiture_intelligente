/*****************************************************************************

Programme permettant d'utiliser le gyroscope MPU6050

*****************************************************************************

Raccordement du capteur :

Vcc -> +5V
Gnd -> Gnd
SCL -> Pin 1/263
SDA -> Pin 0/264

*****************************************************************************

Compilation du programme :

gcc -Wall stabilisation.c lxlib.c -lpthread -o stabilisation

*****************************************************************************/

#include <stdio.h> // pour EXIT_SUCCESS
#include <stdlib.h> // pour printf
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h> // pour open
#include <sys/ioctl.h> // pour ioctl
#include <linux/i2c-dev.h>
#include <pthread.h>
#include <linux/gpio.h>
#include <string.h>
#include "lxlib.h"

#define BOUCLE 271

int stop = 0;

struct pwm{

	struct pin pin;
	pthread_t thread;
	int period;
	int duty_cycle;

};

void *arret( void *arg){

        int *boucle = (int *)arg;
        struct pin arret;
        int value, fd;

        if((fd = InitGpio())<0){
                perror("Erreur : Initialisation Gpio");
                pthread_exit(arg);;
        }

        arret.fd = fd;
        arret.gpio = BOUCLE;
        if(GpioIn(&arret)<0){
                pthread_exit(arg);
        }

	if(GpioRead(&arret)<0){
		perror(" Lecture de la pin d'arret");
		pthread_exit(arg);
	}
	printf("Lecture de la pin d'arret OK\n");

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
        	stop = *boucle;
	}
}

void *SetPwm(void *arg){

	struct pwm *pwm = (struct pwm *)arg;
	struct pin gpio;
	int highsleep,lowsleep;

	gpio = (* pwm).pin;

	highsleep = (*pwm).duty_cycle;
	lowsleep = (*pwm).period - (*pwm).duty_cycle;


	if(GpioWrite(&gpio, 0)<0){
		perror("Teste pwm thread");
		pthread_exit(NULL);
	}
	printf("Début du signal\n");

	while(stop){

		highsleep = (*pwm).duty_cycle;
		lowsleep = (*pwm).period - (*pwm).duty_cycle;
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

int main (int argc, char * argv[]){

        int16_t accel[3]= {10}, gyro, temp;
        int fd_i2c, boucle = 0;
        pthread_t t1, t2;
	struct pin pwm;
	struct pwm pwm1;
	float f_gyro;

        /*Let open the bus*/
        fd_i2c = open("/dev/i2c-1", O_RDWR);

        if(fd_i2c < 0){
                perror("Error openning the i2c Bus");
                return -1;
        }
        printf("Bus Open \n");

        if(pthread_create(&t1, NULL, *arret, &boucle)){
                perror("Error: Creation du thread");
                return -1;
        }
        printf("Thread Creer\n");

        if(ioctl(fd_i2c, I2C_SLAVE, 0x68) < 0){
                perror("Error to setting slave adress");
                close(fd_i2c);
                return -1;
        }

        /* Now access to the bus */
        //Vérification d'acces
        if(verif(&fd_i2c)<0){
                close(fd_i2c);
                return -1;
        }

        // Reset MPU6050
        if(reset(&fd_i2c)!=0){
                perror("Error réinitialisation");
                close(fd_i2c);
                return -1;
        }

        // Configuration MPU6050
        if(config(&fd_i2c)!=0){
                perror("Error configuration");
                close(fd_i2c);
                return -1;
        }

        if(verif(&fd_i2c)<0){
                close(fd_i2c);
                return -1;
        }
	printf("Initialisation I2C fini\n");

	if((pwm.fd = InitGpio()) < 0){
		perror("Erreur : Initialisation Gpio");
		return -1;
	}
	pwm.gpio = PWM1;

	if(GpioOut(&pwm) < 0){
		close(pwm.fd);
          	perror("setting pin du Pwm");
		return -1;
        }
	pwm1.pin = pwm;
	pwm1.duty_cycle = 1466;

	if(InitPwm(&pwm1) < 0){
		close(pwm.fd);
		perror("Initilisation Pwm");
                return -1;
        }
	t2 = pwm1.thread;
	printf("Initialisation Pwm fini\n");
        printf("Initialisation complète fini, lancement du programme\n");
        sleep(1);

	// Read data
	printf(" %d \n", boucle);
        while(boucle){
                if(mpu_read_raw(&fd_i2c, accel, &gyro, &temp)<0){
                        perror("Error reading data");
                        close(fd_i2c);
                        return -1;
                }

                printf(" accel X: %6.3f accel Y: %6.3f\n", ((float)accel[0])/16875.0,((float)accel[1])/-16384.0);
                printf(" gyro X: %6.3f\n",((float)gyro)/65.5);
                printf(" temp: %5.2f\n", (((float)temp) / 340.0) + 36.53);

		f_gyro = ((float)gyro)/65.5;
		if(f_gyro > 160){
			f_gyro = 130;
		}
		else if(f_gyro < -130){
			f_gyro = -130;
		}

		pwm1.duty_cycle = f_gyro*6.15+ 1466;
		printf(" Duty cycle %d \n", pwm1.duty_cycle);
                msleep(50);
        }
        printf("Fermeture du programme ...\n");
        close(fd_i2c);
	pthread_cancel(t1);
	pthread_cancel(t2);
	close(pwm.fd);
        printf("Programme fermé\n");
	msleep(500);
        return 0;
}

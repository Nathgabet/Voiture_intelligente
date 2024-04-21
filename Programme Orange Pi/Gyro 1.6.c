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

gcc -Wall gyroLX.c lxlib.c -lpthread -o gyroLX

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

#define BOUCLE 260

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

int main (int argc, char * argv[]){

        int i2c_bus;
        int16_t accel[3]= {10}, gyro, temp;
        int boucle = 1;
        pthread_t t1;

        /*Let open the bus*/
        i2c_bus = open("/dev/i2c-1", O_RDWR);

        if(i2c_bus < 0){
                perror("Error openning the i2c Bus");
                return -1;
        }
        printf("Bus Open \n");

        if(pthread_create(&t1, NULL, *arret, &boucle)){
                perror("Error: Creation du thread");
                return -1;
        }
        printf("Thread Creer\n");

        if(ioctl(i2c_bus, I2C_SLAVE, 0x68) < 0){
                perror("Error to setting slave adress");
                close(i2c_bus);
                return -1;
        }

        /* Now access to the bus */
        //Vérification d'acces
        if(verif(&i2c_bus)<0){
                close(i2c_bus);
                return -1;
        }

        // Reset MPU6050
        if(reset(&i2c_bus)!=0){
                perror("Error réinitialisation");
                close(i2c_bus);
                return -1;
        }

        // Configuration MPU6050
        if(config(&i2c_bus)!=0){
                perror("Error configuration");
                close(i2c_bus);
                return -1;
        }

        if(verif(&i2c_bus)<0){
                close(i2c_bus);
                return -1;
        }

        printf("Initialisation fini, lancement du programme\n");
        // Read data
        while(boucle){
                if(mpu_read_raw(&i2c_bus, accel, &gyro, &temp)<0){
                        perror("Error reading data");
                        close(i2c_bus);
                        return -1;
                }

                printf(" accel X: %6.3f accel Y: %6.3f\n", ((float)accel[0])/16875.0,((float)accel[1])/-16384.0);
                printf(" gyro X: %6.3f\n",((float)gyro)/65.5);
                printf(" temp: %5.2f\n", (((float)temp) / 340.0) + 36.53);

                msleep(500);
        }
        printf("Fermeture du programme ...\n");
        close(i2c_bus);
        printf("Programme fermé\n");

        return 0;
}

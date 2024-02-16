#include <stdio.h> //de base
#include <stdlib.h> //pour printf
#include <string.h> //pour les chaine de caractere
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>// pour open
#include <sys/ioctl.h>// pour ioctl
#include <linux/gpio.h>// pour struct gpiochip
#include "lxlib.h"

int GpioIn(struct pin *bouton){

        struct gpiohandle_request flag;
        int fd;

        fd = (*bouton).fd;
        flag = (*bouton).handle;

        flag.flags = GPIOHANDLE_REQUEST_INPUT;
        strcpy(flag.consumer_label, "BOUTON");
        memset(flag.default_values, 0, sizeof(flag.default_values));
        flag.lines = 1;
        flag.lineoffsets[0] = (*bouton).gpio;

        if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &flag) < 0) {

                perror("Error setting GPIO to input");
                close(fd);
                return -1;
        }
        else{
                printf("Pin %d INPUT\n", (*bouton).gpio);
                (*bouton).handle = flag;
                return 1;
        }
}

int GpioOut(struct pin *led){

        struct gpiohandle_request flag;
        int fd;

        fd = (*led).fd;
        flag = (*led).handle;

        flag.flags = GPIOHANDLE_REQUEST_OUTPUT;
        strcpy(flag.consumer_label, "LED");
        memset(&flag, 0, sizeof(flag.default_values));
        flag.lines = 1;
        flag.lineoffsets[0] = (*led).gpio;

        if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &flag) < 0) {

                perror("Error setting GPIO to output");
                close(fd);
                return -1;
        }
        else{
                printf("Pin %d OUTPUT\n", (*led).gpio);
                (*led).handle = flag;
                return 1;
        }

}

int GpioWrite(struct pin *led, int value){

        struct gpiohandle_data data;

        data.values[0] = value;

        if(ioctl((*led).handle.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0){
                perror("Error to setting GPIO");
                return -1;
        }

        return 1;
}

int GpioRead(struct pin *bouton){

        struct gpiohandle_data data;

        memset(&data, 0, sizeof(struct gpiohandle_data));
        if(ioctl((*bouton).handle.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0){
                perror("Error to reading GPIO value");
                return -1;
         }
        else{
                return data.values[0];
        }
}


int InitGpio(){

        int fd;

        fd = open("/dev/gpiochip1", O_RDWR);
        if(fd <0){
                perror("Error opening gpiochip1");
                return -1;
         }
        else{
                printf("gpiochip1 Open\n");
        }

        return fd;
}

int msleep(unsigned int tms) {
  return usleep(tms * 1000);
}

int reset(int *i2c_bus){

        uint8_t buf[2] = {0x6B, 0x80};

        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur: Réinitialisation du MPU6050");
                return -1;
        }
        msleep(200);
        printf("Réinitialisation du MPU6050\n" );
        return 0;
}

int config(int *i2c_bus){

        uint8_t buf[2];

        buf[0] = 0x6B;
        buf[1] = 0x00;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur : Reglage de l'alimentation");
                return -1;
        }
        msleep(200);
        printf("Reglage de l'alimentation\n" );

        buf[0] = 0x1A;
        buf[1] = 0x05;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur : Reglage de la synchronisation");
                 return -1;
        }
        msleep(200);
        printf("Reglage de la synchronisation\n" );

        buf[0] = 0x1B;
        buf[1] = 0x08;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur : Reglage du Gyroscope");
                return -1;
        }
        msleep(200);
        printf("Reglage du Gyroscope\n" );

        buf[0] = 0x1C;
        buf[1] = 0x00;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur : Reglage de l'accélérometre");
                return -1;
        }
        msleep(200);
        printf("Reglage de l'accélérometre\n" );

        buf[0] = 0x19;
        buf[1] = 0x07;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur : Reglage du sample rate diviser");
                return -1;
        }
        msleep(200);
        printf("Reglage du sample rate diviser\n" );

        buf[0] = 0x38;
        buf[1] = 0x01;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur : Activation des data");
                return -1;
        }
        msleep(200);
        printf("Activation des data\n" );

        return 0;
}

int mpu_read_raw(int *i2c_bus, int16_t accel[3], int16_t *gyro, int16_t *temp){

        uint8_t data, buffer[6];

        data = 0x3B;
        if(write(*i2c_bus,&data, 1) != 1){
                perror("Error write for accel reading\n");
                return -1;
        }
        msleep(1);
        if(read(*i2c_bus, buffer, 6) !=6){
                perror("Error to read accel\n");
                return -1;
        }
        msleep(30);
        for (int i = 0; i < 3; i++) {
                accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
        }

        data = 0x41;
        if(write(*i2c_bus,&data, 1) != 1){
                perror("Error write for accel reading\n");
                return -1;
        }
        msleep(1);
        if(read(*i2c_bus, buffer, 2) !=2){
                perror("Error to read accel\n");
                return -1;
        }
        msleep(30);
        *temp = buffer[0] << 8 | buffer[1];

        data = 0x47;
        if(write(*i2c_bus,&data, 1) != 1){
                perror("Error write for accel reading\n");
                return -1;
        }
        msleep(1);
        if(read(*i2c_bus, buffer, 2) !=2){
                perror("Error to read accel\n");
                return -1;
        }
        msleep(30);
        *gyro = (buffer[0] << 8 | buffer[1]);

        return 1;
}

int verif(int *i2c_bus){

        uint8_t buf;

        buf =0x75;
        if(write(*i2c_bus, &buf,1) !=1){
                perror("Error writing on the bus");
                return -1;
        }

        if(read(*i2c_bus,&buf,1)!=1){
                perror("Error reading on the bus");
                return -1;
        }
        msleep(1);
        if(buf != 104){
                perror("Erreur de lecture/ecriture de registre\n");
                return -1;
        }
        printf("Lecture/ecriture validé\n");
        return 1;

}

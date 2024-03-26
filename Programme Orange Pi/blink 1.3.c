/*
Programme pour stoper un proggramme grâce à un bouton
de manière à sortir correctement du code par un fonctionnement
en thread

*****************************************************************************
Pour compiler :

        gcc -Wall blthreadLX.c -o blthreadLX

*/

#include <stdio.h> //de base
#include <stdlib.h> //pour printf
#include <string.h> //pour les chaine de caractere
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>// pour open
#include <sys/ioctl.h>// pour ioctl
#include <linux/gpio.h>// pour struct gpiochip

typedef struct pin{
        int fd;
        struct gpiohandle_request handle;
        int gpio;
}pin;

int GpioWrite(pin *led, int value);
int GpioRead(pin *bouton);
int InitGpio();
int msleep(unsigned int tms);
int GpioIn(pin *bouton);
int GpioOut(pin *bouton);

int main (void){

        int fd, raw_bouton;
        struct pin bouton, led;

        if( (fd = InitGpio())<0){
                perror("Erreur : Initialisation Gpio");
                return -1;
        }

        bouton.fd = fd;
        led.fd = fd;

        bouton.gpio = 260;
        led.gpio = 259;

        if(GpioIn(&bouton)<0){
                close(fd);
                return -1;
        }
        if(GpioOut(&led)<0){
                close(fd);
                return -1;
        }

        if(GpioWrite(&led,1)<0){
                close(fd);
                return -1;
        }
        if((raw_bouton=GpioRead(&bouton))<0){
                close(fd);
                return -1;
        }

        msleep(50);

        if(GpioWrite(&led,0)<0){
                close(fd);
                return -1;
        }

        msleep(500);
        close(fd);

        return 0;
}

int GpioIn(pin *bouton){

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

int GpioOut(pin *led){

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

int GpioWrite(pin *led, int value){

        struct gpiohandle_data data;

        data.values[0] = value;

        if(ioctl((*led).handle.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0){
                perror("Error to setting GPIO");
                return -1;
        }

        return 1;
}

int GpioRead(pin *bouton){

        struct gpiohandle_data data;

        memset(&data, 0, sizeof(struct gpiohandle_data));
        if(ioctl((*bouton).handle.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0){
                perror("Error to reading GPIO value");
                return -1;
         }
        else{
                printf("Line %d is %s\n", 259, data.values[0] == 0 ? "LOW" : "HIGH");
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
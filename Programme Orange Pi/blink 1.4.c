/*
Programme pour stoper un proggramme grâce à un bouton
de manière à sortir correctement du code par un fonctionnement
en thread

*****************************************************************************
Pour compiler :

        gcc -Wall blthreadLX.c lxlib.c -o blthreadLX

*/

#include <stdio.h> //de base
#include <stdlib.h> //pour printf
#include <string.h> //pour les chaine de caractere
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>// pour open
#include <sys/ioctl.h>// pour ioctl
#include <linux/gpio.h>// pour struct gpiochip
#include "lxlib.h"


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


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

int BoutonSetup(int pin);

int main (void){

        int fd;
        struct gpiohandle_request bouton;
        struct gpiohandle_data data;

        if(BoutonSetup(260)<0){

                perror("Erreur: Parametrage du bouton");
                return -1;

        }

        /* Reading button's state */
        if(ioctl(bouton.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0){
                perror("Error to reading GPIO value");
        }

        printf("Boutton is %s \n", (data.values[0] > 0) ? "pressed" : "not pressed");
        sleep(2);
        close(fd);

        return 0;
}

int BoutonSetup(int pin){

        int fd;
        struct gpiohandle_request bouton;
        struct gpiohandle_data data;

        fd = open("/dev/gpiochip1", O_RDWR);
        if(fd <0){
                perror("Error opening gpiochip0");
                return -1;
        }

        bouton.flags = GPIOHANDLE_REQUEST_INPUT;
        strcpy(bouton.consumer_label, "LED");
        memset(bouton.default_values, 0, sizeof(bouton.default_values));
        bouton.lines = 1;
        bouton.lineoffsets[0] = pin;

        if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &bouton) < 0) {

                perror("Error setting GPIO 23 to input");
                close(bouton.fd);
                close(fd);
                return -1;
        }

        return 1;
}

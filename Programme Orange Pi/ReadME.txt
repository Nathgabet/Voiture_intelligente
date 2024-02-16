
Pour programmer grâce au kernel suivre la structure suivante :
*****************************************************************************
#include <stdio.h> // pour EXIT_SUCCESS
#include <stdlib.h> // pour printf
#include <linux/gpio.h> // pour struct gpiochip_info
#include <fcntl.h> // pour open
#include <sys/ioctl.h> // pour ioctl
#include <string.h> // Pour memset

int main (int argc, char * argv[]){





        return 0;
}
*****************************************************************************

Pour compiler et executer:

        gcc -Wall "nom_programme.c" -o "nom_executable(.bin)"
        sudo ./"nom_executable(.bin)

*****************************************************************************

Ouvrir le /dev/gpiochip1 pas le 0

*****************************************************************************

Utiliser ces fonctions et structures:

        - perror(" texte ");//pour voir le type d'erreur
        - struct gpiohandle_request {
                __u32 lineoffsets[GPIOHANDLES_MAX];
                __u32 flags;
                __u8 default_values[GPIOHANDLES_MAX];
                char consumer_label[32];
                __u32 lines;
                int fd;
         };//pour des commande sur les pins
        - struct gpiohandle_data {
                __u8 values[GPIOHANDLES_MAX];
         };//permet de stocker l'etat de la pin
*****************************************************************************

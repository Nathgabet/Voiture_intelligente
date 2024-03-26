/*

Pour compiler :

        gcc -Wall blinkLX.c -o blinkLX

*/

#include <stdio.h> //de base
#include <stdlib.h> //pour printf
#include <string.h> //pour les chaine de caractere
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>// pour open
#include <sys/ioctl.h>// pour ioctl
#include <linux/gpio.h>// pour struct gpiochip


int main (void){

        int fd;
        struct gpiohandle_request led, bouton;
        struct gpiohandle_data data;

        /* Open the handle device file */
        fd = open("/dev/gpiochip1", O_RDWR);
        if(fd <0){
                perror("Error opening gpiochip0");
                return -1;
        }

        /* setup LED OUTPUT */
        led.flags = GPIOHANDLE_REQUEST_OUTPUT;
        strcpy(led.consumer_label, "LED");
        memset(&led, 0, sizeof(led.default_values));
        led.lines = 1;
        led.lineoffsets[0] = 268;

        if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &led) < 0) {

                perror("Error setting GPIO 22 to output");
                close(fd);
                return -1;
        }

        memset(&data, 0, sizeof(struct gpiohandle_data));
        ioctl(led.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
        printf("Line %d is %s\n", 226, data.values[0] == 0 ? "LOW" : "HIGH");

         /* setup LED INPUT */
        bouton.flags = GPIOHANDLE_REQUEST_INPUT;
        strcpy(bouton.consumer_label, "LED");
        memset(bouton.default_values, 0, sizeof(bouton.default_values));
        bouton.lines = 1;
        bouton.lineoffsets[0] = 227;

        if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &bouton) < 0) {

                perror("Error setting GPIO 23 to input");
                close(led.fd);
                close(fd);
                return -1;
        }

        /* Let set the LED */
        data.values[0] = 1;
        if(ioctl(led.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0){
                perror("Error to setting GPIO 22 to 1");
        }

        /* Reading button's state */
        if(ioctl(bouton.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0){
                perror("Error to reading GPIO value");
        }

        printf("Boutton is %s \n", (data.values[0] > 0) ? "pressed" : "not pressed");

        sleep(2);//sleep in sec

        data.values[0] = 0;
        if(ioctl(led.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0){
                perror("Error to setting GPIO 22 to 0");
        }

        sleep(2);

        close(fd);

        return 0;
}
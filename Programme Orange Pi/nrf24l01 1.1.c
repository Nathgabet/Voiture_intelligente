#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <linux/gpio.h>
#include "lxlib.h"

int main (int argc, char *argv[]){

        int fd_spi, fd_gpio;
        uint8_t data[3], mode = 0, bits = 8;
        struct pin led_tx, led_rx, cs;
        uint32_t speed = 500000;

        //Init Gpio
        if((fd_gpio = InitGpio())<0){
                return -1;
        }
        printf("Gpio init \n");

        //Setting Pin
        cs.fd = fd_gpio;
        led_tx.fd = fd_gpio;
        led_rx.fd = fd_gpio;
        cs.gpio = 233;
        led_tx.gpio = 259;
        led_rx.gpio = 268;

        if(GpioOut(&led_rx)<0){
                close(fd_gpio);
                return -1;
        }
        printf(" Led Rx Set \n");
        GpioWrite(&led_rx, 0);

        if(GpioOut(&led_tx)<0){
                close(fd_gpio);
                return -1;
        }
        printf(" Led Tx Set \n");
        GpioWrite(&led_tx, 0);

        if(GpioOut(&cs)<0){
                close(fd_gpio);
                return -1;
        }
        printf(" Cs Set \n");
        GpioWrite(&cs, 0);

        //open SPI Bus
        if((fd_spi = open("/dev/spidev1.0", O_RDWR)) <0){
                perror("Erreur : Open SPI Bus");
                close(fd_gpio);
                return -1;
        }
        printf("SPI Bus open \n");

        //Setup of SPI Bus
        if(ioctl(fd_spi, SPI_IOC_WR_MODE, &mode) <0){
                perror("Erreur : Setting mode");
                close(fd_spi);
                close(fd_gpio);
                return -1;
        }
        if(ioctl(fd_spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed) <0){
                perror("Erreur : Setting speed");
                close(fd_spi);
                close(fd_gpio);
                return -1;
        }
        if(ioctl(fd_spi, SPI_IOC_WR_BITS_PER_WORD, &bits) <0){
                perror("Erreur : Setting wordlengh");
                close(fd_spi);
                close(fd_gpio);
                return -1;
        }
        printf(" Spi Bus set\n");

        GpioWrite(&led_tx, 1);
        if(nrf24Reset(fd_spi, 0)<0){
                perror("Error: Reset nrf24");
                close(fd_gpio);
                close(fd_spi);
                return -1;
        }
        GpioWrite(&led_tx, 0);
        printf("nrf24 Reset \n");
        msleep(100);

        GpioWrite(&led_tx, 1);
        if(nrf24Init(fd_spi)<0){
                perror("Error: Init nrf24");
                close(fd_gpio);
                close(fd_spi);
                return -1;
        }
        GpioWrite(&led_tx, 0);
        printf("nrf24 Init \n");



        msleep(100);
        close(fd_gpio);
        close(fd_spi);

        return 0;
}
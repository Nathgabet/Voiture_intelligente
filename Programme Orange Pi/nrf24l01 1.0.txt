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

uint8_t mode = 0;
uint32_t speed = 500000;
uint8_t bits = 8;

int spi_transfer(int fd, uint8_t *data, int lenght){

        struct spi_ioc_transfer spi[lenght];
        int i;

        for( i =0; i<lenght; i++){
                memset(&spi[i], 0, sizeof(struct spi_ioc_transfer));
                spi[i].tx_buf = (unsigned long) (data + i);
                spi[i].rx_buf = (unsigned long) (data + i);
                spi[i].len = 1;
                spi[i].speed_hz = speed;
                spi[i].bits_per_word = bits;
        }

        if(ioctl(fd, SPI_IOC_MESSAGE(lenght), spi) < 0){
                perror("Error : transfert data to SPI bus");
                close(fd);
                return -1;
        }

        return 1;

}


int main (int argc, char *argv[]){

        int fd_spi, fd_gpio;
        uint8_t data[3];
        struct pin led_tx, led_rx, cs;

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
        printf(" Mode set \n");

        if(ioctl(fd_spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed) <0){
                perror("Erreur : Setting speed");
                close(fd_spi);
                close(fd_gpio);
                return -1;
        }
        printf(" Speed set \n");

        if(ioctl(fd_spi, SPI_IOC_WR_BITS_PER_WORD, &bits) <0){
                perror("Erreur : Setting wordlengh");
                close(fd_spi);
                close(fd_gpio);
                return -1;
        }
        printf(" Wordlengh set \n");

        data[0] = 0x40;
        data[1] = 0x00;
        data[2] = 0xFE;

        GpioWrite(&led_tx, 1);
        if(spi_transfer(fd_spi, data, 3)<0){
                perror("Error: tranfers data");
                close(fd_gpio);
                close(fd_spi);
                return -1;
        }
        GpioWrite(&led_tx, 0);

        close(fd_gpio);
        close(fd_spi);

        return 0;
}

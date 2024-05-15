#include <stdio.h> //de base
#include <stdlib.h> //pour printf
#include <string.h> //pour les chaine de caractere
#include <stdint.h>//Permet d'utiliser les variables non signees
#include <unistd.h>
#include <fcntl.h>// pour open
#include <sys/ioctl.h>// pour ioctl
#include <linux/gpio.h>// pour struct gpiochip
#include <linux/spi/spidev.h>//permet d'utiliser le bus SPI
#include <linux/joystick.h>//permet d'utiliser la manette
#include <pthread.h>
#include "lxlib.h"

/**
 * Fonction associes au port GPIO
 * Declare un port GPIO comme entree
*/
int GpioIn(struct pin *bouton){

        struct gpiohandle_request flag;
        int fd;

        fd = (*bouton).fd;
        flag = (*bouton).handle;

        flag.flags = GPIOHANDLE_REQUEST_INPUT;
        strcpy(flag.consumer_label, "BOUTON");
        memset(flag.default_values, 0, sizeof(flag.default_values));        flag.lines = 1;
        flag.lineoffsets[0] = (*bouton).gpio;

        if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &flag) < 0) {

                perror("Erreur: Definir le port GPIO en INPUT");
                close(fd);
                return -1;
        }
        else{
                printf("Pin %d déclarée en INPUT\n", (*bouton).gpio);
                (*bouton).handle = flag;
                return 1;
        }
}

/**
 * Declare un port GPIO comme sortie
*/
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

                perror("Erreur: Definir le port GPIO en output");
                close(fd);
                return -1;
        }
        else{
                printf("Pin %d déclarée en OUTPUT\n", (*led).gpio);
                (*led).handle = flag;
                return 1;
        }

}

/**
 * permet de changer l'etat d'une sortie
*/
int GpioWrite(struct pin *led, int value){

        struct gpiohandle_data data;

        data.values[0] = value;

        if(ioctl((*led).handle.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0){
                perror("Erreur: Affectation sur GPIO");
                return -1;
        }

        return 1;
}
/**
 * Permet de lire l'etat d'une entree
*/
int GpioRead(struct pin *bouton){

        struct gpiohandle_data data;

        memset(&data, 0, sizeof(struct gpiohandle_data));
        if(ioctl((*bouton).handle.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0){
                perror("Erreur: Lecture du port ");
                return -1;
         }
        else{
                return data.values[0];
        }
}
/**
 * Initialise les ports GPIO
*/
int InitGpio(){

        int fd;

        fd = open("/dev/gpiochip1", O_RDWR);
        if(fd <0){
                perror("Erreur: Ouverture du port Gpio");
                return -1;
        }
        else{
                printf("Port Gpio Ouvert\n");
        }

        return fd;
}
/**
 * Fonction Permettant de faire des attentes en ms
*/
int msleep(unsigned int tms) {
  return usleep(tms * 1000);
}
/**
 * long distance_raw - fonction permettant de calculer la distance
*/
long distance_raw(struct pin *echo){

        struct timeval start, stop;

        while(GpioRead(echo) == LOW){
               gettimeofday(&start, NULL);
        }
        while(GpioRead(echo) == HIGH){
            gettimeofday(&stop, NULL);

        }

       return ((stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec)*0.017);

}

/**
 * Fonction associes à la communication I2C
*/
int Initi2c(){

        int fd;

        if( (fd = open("/dev/i2c-1", O_RDWR)) < 0){
                perror("Erreur : Ouverture du port i2c");
                return -1;
        }
        printf("Port i2c ouvert\n");

        return fd;
}
/**
 * Permet de reinitialiser le gyroscope
*/
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
/**
 * Permet de configurer le Gyroscope
*/
int config(int *i2c_bus){

        uint8_t buf[2];

        buf[0] = 0x6B;
        buf[1] = 0x00;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur: Reglage de l'alimentation");
                return -1;
        }
        msleep(200);
        printf("Reglage de l'alimentation\n" );

        buf[0] = 0x1A;
        buf[1] = 0x05;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur: Reglage de la synchronisation");
                 return -1;
        }
        msleep(200);
        printf("Reglage de la synchronisation\n" );

        buf[0] = 0x1B;
        buf[1] = 0x08;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur: Reglage du Gyroscope");
                return -1;
        }
        msleep(200);
        printf("Reglage du Gyroscope\n" );

        buf[0] = 0x1C;
        buf[1] = 0x00;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur: Reglage de l'accélérometre");
                return -1;
        }
        msleep(200);
        printf("Reglage de l'accélérometre\n" );

        buf[0] = 0x19;
        buf[1] = 0x07;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur: Reglage du sample rate diviser");
                return -1;
        }
        msleep(200);
        printf("Reglage du sample rate diviser\n" );

        buf[0] = 0x38;
        buf[1] = 0x01;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur: Activation des datas");
                return -1;
        }
        msleep(200);
        printf("Activation des datas\n" );

        return 0;
}
/**
 * Permet de lire les valeurs du gyroscope
*/
int mpu_read_raw(int *i2c_bus, int16_t accel[3], int16_t *gyro, int16_t *temp){

        uint8_t data, buffer[6];

        data = 0x3B;
        if(write(*i2c_bus,&data, 1) != 1){
                perror("Erreur: Ecriture d'addr pour la lecture de l'accel\n");
                return -1;
        }
        msleep(1);
        if(read(*i2c_bus, buffer, 6) !=6){
                perror("Erreur: Lecture accel\n");
                return -1;
        }
        msleep(5);
        for (int i = 0; i < 3; i++) {
                accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
        }

        data = 0x41;
        if(write(*i2c_bus,&data, 1) != 1){
                perror("Erreur: Ecriture d'addr pour la lecture de l'accel\n");
                return -1;
        }
        msleep(1);
        if(read(*i2c_bus, buffer, 2) !=2){
                perror("Erreur: Lecture temp\n");
                return -1;
        }
        msleep(5);
        *temp = buffer[0] << 8 | buffer[1];

        data = 0x43;
        if(write(*i2c_bus,&data, 1) != 1){
                perror("Erreur:Ecriture d'addr pour la lecture de l'accel\n");
                return -1;
        }
        msleep(1);
        if(read(*i2c_bus, buffer, 2) !=2){
                perror("Erreur: Lecture gyro\n");
                return -1;
        }
        msleep(5);
        *gyro = (buffer[0] << 8 | buffer[1]);

        return 1;
}
/**
 * Permet de verifier le bon fonctionnement du gyroscope
*/
int verif(int *i2c_bus){

        uint8_t buf;

        buf =0x75;
        if(write(*i2c_bus, &buf,1) !=1){
                perror("Erreur: Ecrire sur le bus");
                return -1;
        }

        if(read(*i2c_bus,&buf,1)!=1){
                perror("Erreur: Lire sur le bus");
                return -1;
        }
	msleep(1);
        if(buf != 104){
                perror("Erreur: de lecture/ecriture de registre\n");
                return -1;
        }
        printf("Lecture/ecriture validé\n");
        return 1;

}
/**
 * Fonctions associe au Bus SPI
 * Permet le tranfert de donnees
*/
int spi_transfer(int fd, uint8_t *data, int lenght){

        struct spi_ioc_transfer spi[lenght];
        int i;
        uint32_t speed = 500000;
        uint8_t bits = 8;

        for( i =0; i<lenght; i++){
                memset(&spi[i], 0, sizeof(struct spi_ioc_transfer));
                spi[i].tx_buf = (unsigned long) (data + i);
                spi[i].rx_buf = (unsigned long) (data + i);
                spi[i].len = 1;
                spi[i].speed_hz = speed;
                spi[i].bits_per_word = bits;
        }

        if(ioctl(fd, SPI_IOC_MESSAGE(lenght), spi) < 0){
                perror("Erreur: Avoit des donnée au bus SPI");
                close(fd);
                return -1;
        }

        return 1;

}
/**
 * Initialise le transmeteur NRF24
*/
int  nrf24Init(int fd){

         if(nrf24TransferRegister(fd,CONFIG,0x00)<0){
                        perror("Erreur: Init: CONFIG ");
                        return -1;
                }
         if(nrf24TransferRegister(fd,EN_AA,0x00)<0){
                        perror("Erreur: Init: EN_AA ");
                        return -1;
                }
         if(nrf24TransferRegister(fd,EN_RXADDR,0x00)<0){
                        perror("Erreur: Init: EN_RXADDR ");
                        return -1;
                }
         if(nrf24TransferRegister(fd,SETUP_AW,0x03)<0){
                        perror("Erreur: Init: SETUP_AW ");
                        return -1;
                }
         if(nrf24TransferRegister(fd,SETUP_RETR,0x00)<0){
                        perror("Erreur: Init: SETUP_RETR ");
                        return -1;
                }
         if(nrf24TransferRegister(fd,RF_CH,0x00)<0){
                        perror("Erreur: Init: RF_CH ");
                        return -1;
                }
         if(nrf24TransferRegister(fd,RF_SETUP,0x0E)<0){
                        perror("Erreur: Init: RF_SETUP ");
                        return -1;
                }

        return 1;
}
/**
 * Reinitialiser le transmeteur
*/
int nrf24Reset(int fd, uint8_t reg){

        if(reg == STATUS){
                if(nrf24TransferRegister(fd,STATUS,0x00)<0){
                        perror("Erreur: Reset: STATUS ");
                        return -1;
                }
        }
        else if(reg == FIFO_STATUS){
                if(nrf24TransferRegister(fd,FIFO_STATUS,0x00)<0){
                        perror("Erreur: Reset: FIFO STATUS ");
                        return -1;
                }
        }
        else{
                uint8_t rx_addr_p0_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
                uint8_t rx_addr_p1_def[5] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2};
                uint8_t tx_addr_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
                nrf24TransferRegister(fd,CONFIG, 0x08);
                nrf24TransferRegister(fd,EN_AA, 0x3F);
                nrf24TransferRegister(fd,EN_RXADDR, 0x03);
                nrf24TransferRegister(fd,SETUP_AW, 0x03);
                nrf24TransferRegister(fd,SETUP_RETR, 0x03);
                nrf24TransferRegister(fd,RF_CH, 0x02);
                nrf24TransferRegister(fd,RF_SETUP, 0x0E);
                nrf24TransferRegister(fd,STATUS, 0x00);
                nrf24TransferRegister(fd,OBSERVE_TX, 0x00);
                nrf24TransferRegister(fd,CD, 0x00);
                nrf24TransferMultiRegister(fd,RX_ADDR_P0, rx_addr_p0_def, 5);
                nrf24TransferMultiRegister(fd,RX_ADDR_P1, rx_addr_p1_def, 5);
                nrf24TransferRegister(fd,RX_ADDR_P2, 0xC3);
                nrf24TransferRegister(fd,RX_ADDR_P3, 0xC4);
                nrf24TransferRegister(fd,RX_ADDR_P4, 0xC5);
                nrf24TransferRegister(fd,RX_ADDR_P5, 0xC6);
                nrf24TransferMultiRegister(fd,TX_ADDR, tx_addr_def, 5);
                nrf24TransferRegister(fd,RX_PW_P0, 0);
                nrf24TransferRegister(fd,RX_PW_P1, 0);
                nrf24TransferRegister(fd,RX_PW_P2, 0);
                nrf24TransferRegister(fd,RX_PW_P3, 0);
                nrf24TransferRegister(fd,RX_PW_P4, 0);
                nrf24TransferRegister(fd,RX_PW_P5, 0);
                nrf24TransferRegister(fd,FIFO_STATUS, 0x11);
                nrf24TransferRegister(fd,DYNPD, 0);
                nrf24TransferRegister(fd,FEATURE, 0);
        }

        return 1;
}
/**
 * Permet de simplifer l'écriture sur le bus SPI
*/
int nrf24TransferRegister(int fd, uint8_t reg, uint8_t data){

        uint8_t buf[2];
        buf[0] = reg|1<<5 ;
        buf[1] = data ;

        if(spi_transfer(fd, buf, 2)<0){
                perror("Erreur: Transfer Registres ");
                return -1;
        }

        return 1;
}
/**
 * Permet de modifier plusuieur registre en une fois
*/
int nrf24TransferMultiRegister(int fd, uint8_t reg, uint8_t *data, int size){

        uint8_t buf[2];
        buf[0] = reg|1<<5 ;
        spi_transfer(fd, buf, 1);

        if(spi_transfer(fd, data, size)<0){
                perror("Erreur: Transfer MultiRegistre ");
                return -1;
        }

        return 1;
}
/**
 * Parametre le Transmeteur en Emeteur
*/
int nrf24TxMode(int fd, uint8_t *adress, uint8_t channel){

        uint8_t data = 0;

        nrf24TransferRegister(fd, RF_CH, channel);
        nrf24TransferMultiRegister(fd, TX_ADDR, adress, 5);

        nrf24TransferRegister(fd, CONFIG, data);
        data = data & (0xF2);

        nrf24TransferRegister(fd, CONFIG, data);

        return 1;
}
/**
 * Parametre le transmetteur en Recepteur
*/
int nrf24RxMode(int fd, uint8_t *adress, uint8_t channel){

        uint8_t config = 0, en_rxaddr = 0;

        nrf24TransferRegister(fd, RF_CH, channel);
        nrf24TransferRegister(fd, EN_RXADDR, en_rxaddr);
        en_rxaddr = en_rxaddr | (1<<2);
        nrf24TransferRegister(fd, EN_RXADDR, en_rxaddr);

        nrf24TransferMultiRegister(fd, RX_ADDR_P1, adress, 5);
        nrf24TransferRegister(fd, RX_PW_P1, 32);

        nrf24TransferRegister(fd, CONFIG, config);
        config = config | (1<<1) | (1<<0);
        nrf24TransferRegister(fd, CONFIG, config);

        return 1;
}
/**
 * Permet de transmettre les donnees avec le NRF24
*/
uint8_t nrf24Transmit(int fd, uint8_t *data){

        uint8_t cmdtosend = 0, fifo_status = 0;

        cmdtosend = W_TX_PAYLOAD;

         if(spi_transfer(fd, &cmdtosend, 1)<0){
                perror("Erreur: Transmit : W_TX_PAYLOAD ");
                return -1;
        }
        msleep(10);
        if(spi_transfer(fd, data, 32)<0){
                perror("Erreur: Transmit : Longueur de data");
                return -1;
        }
        msleep(10);

        nrf24TransferRegister(fd, FIFO_STATUS, fifo_status);

        printf(" -%d \n ", fifo_status);

        if((fifo_status >>4) == 2){

                nrf24TransferRegister(fd, FIFO_STATUS, 0x00);

                if(spi_transfer(fd,&cmdtosend , 1)<0){
                        perror("Erreur: Transmit : FLUSH_TX ");
                        return -1;
                }


                return 1;
        }
        else{
                perror("Erreur: Transmit: Taille fifo_status ");
                return -1;
        }
}
/**
 * Permet de savoir si il y a des donnees à recuperer
*/
uint8_t IsDataAvailable(int fd, int pinenum){

        uint8_t status = 0;

        nrf24TransferRegister(fd, STATUS, status);

        if( (status&(1<<6)) && (status&(pinenum<<1)) ){

                nrf24TransferRegister(fd, STATUS, (1<<6));
                return 1;
        }

        return -1;
}
/**
 * Permet de recevoir les donnees
*/
int nrf24_Receive(int fd, uint8_t *data){

        uint8_t cmdtosend;

        cmdtosend = W_TX_PAYLOAD;

        if(spi_transfer(fd, &cmdtosend, 1)<0){
                perror("Erreur: Receive : W_TX_PAYLOAD ");
                return -1;
        }
        msleep(10);
        if(spi_transfer(fd, data, 32)<0){
                perror("Erreur: Receive : Longueur Data ");
                return -1;
        }
        msleep(10);
        cmdtosend = FLUSH_RX;
        if(spi_transfer(fd,&cmdtosend , 1)<0){
                perror("Erreur: Receive : FLUSH_RX ");
                return -1;
        }

        return 1;
}
/**
 * Fonctions associes a la manette
 * read_event - Fonction permettant de lire les evenements
*/
int read_event(int fd, struct js_event *event){

        ssize_t bytes;

        bytes = read(fd, event, sizeof(*event));

        if (bytes == sizeof(*event))
                return 0;


        return -1;
}
/**
 * get_axis_count - Permet de connetre le nombre axes
*/
size_t get_axis_count(int fd){

        uint8_t axes;

        if (ioctl(fd, JSIOCGAXES, &axes) == -1)
                return 0;

        return axes;
}
/**
 * get_button_count - Permet de connetre le nombre de boutons
*/
size_t get_button_count(int fd){

        uint8_t  buttons;

        if (ioctl(fd, JSIOCGBUTTONS, &buttons) < 0)
                return 0;

        return buttons;
}
/**
 * get_axis_state - Permet de connetre le position des joysticks
*/
size_t get_axis_state(struct js_event *event, struct axis_state axes[3]){

        size_t axis = event->number / 2;

        if (axis < 3){

                if (event->number % 2 == 0)
                        axes[axis].x = event->value;
                else
                        axes[axis].y = event->value;
        }

    return axis;
}
/*
 * *SetPwm - Permet de lancer une Pin en fonctionnent PWM
 * il va changer le rapport cyclique selon les valeurs de la période
 * et du duty cycle
*/
void *SetPwm(void *arg){

       	pwm (*pwmpin) = (pwm*)arg;
        struct pin gpio;
        int highsleep,lowsleep;

        gpio = (* pwmpin).pin;

        highsleep = (*pwmpin).duty_cycle;
        lowsleep = (*pwmpin).period - (*pwmpin).duty_cycle;
	if(GpioWrite(&gpio, 0)<0){
        	perror("Erreur: Affectation GPIO dans le thread PWM");
                pthread_exit(NULL);
        }
        printf("Affectation dans le thread PWM\n");

        while(1){

                highsleep = (*pwmpin).duty_cycle;
                lowsleep = (*pwmpin).period - (*pwmpin).duty_cycle;
                GpioWrite(&gpio,1);
                usleep(highsleep);
                GpioWrite(&gpio,0);
                usleep(lowsleep);

        }

        pthread_exit(NULL);
}

/**
 * InitPwm - Permet de définir une pin en fonctionnment PWM
 * Renvoi 1 en cas de succées sinon -1
 * Fréquence ~=50Hz
*/
int InitPwm(pwm *pwm1){

	pthread_t t1 = (*pwm1).thread ;
	(*pwm1).period = 19750; //period en µs ajousté

	if(pthread_create(&t1, NULL, *SetPwm, pwm1)){
        	perror("Erreur: Creation du thread");
		return -1;
    	}
	printf("Thread créer \n");

	return 1;
}

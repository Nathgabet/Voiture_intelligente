#ifndef LXLIB_H
#define LXLIB_H

/**
 * struct pin - parametre associé à une pin
 * @fd : file descriptor de l'ouverture de gpiochip1
 * @handle : parametre permettant de configurer la pin
 * @gpio : numero associé à la pin
*/
struct pin{
        int fd;
        struct gpiohandle_request handle;
        int gpio;
};

int GpioWrite(struct pin *led, int value);
int GpioRead(struct pin *bouton);
int InitGpio();
int msleep(unsigned int tms);
int GpioIn(struct pin *bouton);
int GpioOut(struct pin *bouton);

long distance_raw(struct pin *echo);

int Initi2c();
int reset(int *i2c_bus);
int config(int *i2c_bus);
int mpu_read_raw(int *i2c_bus, int16_t accel[3], int16_t *gyro, int16_t *temp);
int verif(int *i2c_bus);

#endif

#ifndef TRUE

#define TRUE (1==1)
#define FALSE (!TRUE)
#define HIGH 1
#define LOW 0

#endif
#ifndef NRF24CONFIG
#define NRF24CONFIG

//Liste des registre
#define CONFIG 0x00
#define EN_AA 0x01
#define EN_RXADDR 0x02
#define SETUP_AW 0x03
#define SETUP_RETR 0x04
#define RF_CH 0x05
#define RF_SETUP 0x06
#define STATUS 0x07
#define OBSERVE_TX 0x08
#define CD 0x09
#define RX_ADDR_P0 0x0A
#define RX_ADDR_P1 0x0B
#define RX_ADDR_P2 0x0C
#define RX_ADDR_P3 0x0D
#define RX_ADDR_P4 0x0E
#define RX_ADDR_P5 0x0F
#define TX_ADDR 0x10
#define RX_PW_P0 0x11
#define RX_PW_P1 0x12
#define RX_PW_P2 0x13
#define RX_PW_P3 0x14
#define RX_PW_P4 0x15
#define RX_PW_P5 0x16
#define FIFO_STATUS 0x17
#define DYNPD 0x1C
#define FEATURE 0x1D

//Liste des instruction mnemonique

#define R_REGISTER 0x00
#define W_REGISTER 0x20
#define REGISTER_MASK 0x1F
#define ACTIVATE 0x50
#define R_RX_PL_WID 0x60
#define R_RX_PAYLOAD 0x61
#define W_TX_PAYLOAD 0xA0
#define W_ACK_PAYLOAD 0xA8
#define FLUSH_TX 0XE1
#define FLUSH_RX 0xE2
#define REUSE_RX_PL 0xE3
#define RF24_NOP 0xFF

//Listre des fonciotns du SPI
int spi_transfer(int fd, uint8_t *data, int lenght);
int  nrf24Init(int fd);
int nrf24Reset(int fd, uint8_t reg);
int nrf24TransferRegister(int fd, uint8_t reg, uint8_t data);
int nrf24TransferMultiRegister(int fd, uint8_t reg, uint8_t *data, int size);
int nrf24TxMode(int fd, uint8_t *adress, uint8_t channel);
int nrf24RxMode(int fd, uint8_t *adress, uint8_t channel);
uint8_t nrf24Transmit(int fd, uint8_t *data);
uint8_t IsDataAvailable(int fd, int pinenum);
int nrf24_Receive(int fd, uint8_t *data);

#endif

#ifndef CONTROLLER
#define CONTROLLER

/**
 * struct axis_state - regroupe les deux axes de direction
 * @x : valeur sur l'axe x
 * @y : valeur sur l'axe y
*/
struct axis_state {

        short x, y;

};

int read_event(int fd, struct js_event *event);
size_t get_axis_count(int fd);
size_t get_button_count(int fd);
size_t get_axis_state(struct js_event *event, struct axis_state axes[3]);

#endif

#ifndef PWM
#define PWM

// pin PWM naturel
#define PWM1 267
#define PWM2 268
#define PWM3 269
#define PWM4 270

/**
 * struct pwm - structure contenant les parametres permettant de définir
 * une pin en fonctionnement pwm 
 * @pin : parametres global de fonctionnement d'une pin
 * @thread : adresse du thread associé au pwm
 * @period : durée en micosecond de la periode
 * @duty_cycle : durée de l'état haut du rapport de cycle
*/
typedef struct {

	struct pin pin;
	pthread_t thread;
	int period;
	int duty_cycle;

}pwm;
void *SetPwm(void *arg);
int InitPwm(pwm *pwm1);

#endif

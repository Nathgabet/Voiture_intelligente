#ifndef LXLIB_H
#define LXLIB_H


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

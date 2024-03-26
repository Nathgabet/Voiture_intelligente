#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#define addr 0x68

static void read_raw(uint16_t accel[3],uint16_t gyro[3] ,uint16_t *temp );
static int reset();
static int config();

int main (int argc, char **argv){

        int fd = wiringPiI2CSetup(addr);
        uint16_t accel[3], gyro[3], temp;

        if (fd == -1) {
                printf("Failed to init I2C communication.\n");
                return -1;
        }
        printf("I2C communication successfully setup.\n");

/*      if(reset() !=1){
                perror("Erreur lors deu reset du gyro");
                return -1;
        }*/
        if(config() <0){
                perror("Erreur lors de la configuration du gyro");
                return -1;
        }

        read_raw(accel, gyro, &temp);

        return 0;
}

static void read_raw(uint16_t accel[3],uint16_t gyro[3] ,uint16_t *temp ){

        uint8_t buffer[2];

        buffer[0] = wiringPiI2CReadReg8(addr, 0x3B);
        buffer[1] = wiringPiI2CReadReg8(addr, 0x3C);
        accel[0] = (buffer[0] << 8 | buffer[1]);
        buffer[0] = wiringPiI2CReadReg8(addr, 0x3D);
        buffer[1] = wiringPiI2CReadReg8(addr, 0x3E);
        accel[1] = (buffer[0] << 8 | buffer[1]);
        buffer[0] = wiringPiI2CReadReg8(addr, 0x3F);
        buffer[1] = wiringPiI2CReadReg8(addr, 0x40);
        accel[2] = (buffer[0] << 8 | buffer[1]);



}

static int reset(){

        if(wiringPiI2CWriteReg8( addr, 0x6B, 0x80) !=1){
                perror("Erreur: Réinitialisation du MPU6050");
                return -1;
        }
        printf("Réinitialisation du MPU6050\n" );

        return 1;

}

static int config(){

        if(wiringPiI2CWriteReg8( addr, 0x6B, 0x00)<0){
                perror("Erreur :Reglage de l'alimentation");
                return -1;
        }
        printf("Reglage de l'alimentation\n" );


        if(wiringPiI2CWriteReg8( addr, 0x1A, 0x05)<0){
                perror("Erreur :Reglage de la synchronisation");
                return -1;
        }
        printf("Reglage de la synchronisation\n" );

        if(wiringPiI2CWriteReg8( addr, 0x1B, 0x08)<0){
                perror("Erreur :Reglage du gyroscope");
                return -1;
        }
        printf("Reglage du gyroscope\n" );

        if(wiringPiI2CWriteReg8( addr, 0x1C, 0x00)<0){
                perror("Erreur : Reglage de l'accelerometre");
                return -1;
        }
        printf("Reglage de l'accelerometre\n" );

        if(wiringPiI2CWriteReg8( addr, 0x19, 0x07)<0){
                perror("Erreur :Reglage du sample rate diviser");
                return -1;
        }
        printf("Reglage du sample rate diviser\n" );

        return 1;

}
#include <stdio.h> // pour EXIT_SUCCESS
#include <stdlib.h> // pour printf
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h> // pour open
#include <sys/ioctl.h> // pour ioctl
#include <linux/i2c-dev.h>
#include <wiringPi.h>
#define LED 26

static int reset(int *i2c_bus);
static int config(int *i2c_bus);
static int mpu_read_raw(int *i2c_bus, int16_t accel[3], int16_t *gyro, int16_t *temp);
static int verif(int *i2c_bus);
int boucle = 1;

PI_THREAD (flag){

        int value =0;

        for (;;){
                value = digitalRead (LED) ;  // On
                delay (10) ;               // mS
                if (value == 1){
                        boucle = 0;
                        return 0;
                }
                else{
                        boucle = 1;
                }
        }
}

int main (int argc, char * argv[]){

        int i2c_bus;
        int16_t accel[3]= {10}, gyro, temp;

        wiringPiSetup();

        pinMode (LED, INPUT) ;

        /*Let open the bus*/
        i2c_bus = open("/dev/i2c-1", O_RDWR);

        if(i2c_bus < 0){
                perror("Error openning the i2c Bus");
                return -1;
        }
        printf("Bus Open \n");

        piThreadCreate (flag) ;

        if(ioctl(i2c_bus, I2C_SLAVE, 0x68) < 0){
                perror("Error to setting slave adress");
                close(i2c_bus);
                return -1;
        }

        /* Now access to the bus */
        //Vérification d'acces
        if(verif(&i2c_bus)<0){
                close(i2c_bus);
                return -1;
        }

        // Reset MPU6050
        if(reset(&i2c_bus)!=0){
                perror("Error réinitialisation");
                close(i2c_bus);
                return -1;
        }

        // Configuration MPU6050
        if(config(&i2c_bus)!=0){
                perror("Error configuration");
                close(i2c_bus);
                return -1;
        }

        if(verif(&i2c_bus)<0){
                close(i2c_bus);
                return -1;
        }

        // Read data
        while(boucle){
                if(mpu_read_raw(&i2c_bus, accel, &gyro, &temp)<0){
                        perror("Error reading data");
                        close(i2c_bus);
                        return -1;
                }

                printf(" accel X: %6.3f accel Z: %6.3f\n", ((float)accel[0])/16875.0,((float)accel[2])/17940.48);
                printf(" gyro Z: %6.3f\n",((float)gyro)/65.5);
                printf(" temp: %5.2f\n", (((float)temp) / 340.0) + 36.53);

                delay(500);
        }
        printf("Fermeture du programme ...\n");
        close(i2c_bus);
        printf("Programme fermé\n");

        return 0;
}

static int reset(int *i2c_bus){

        uint8_t buf[2] = {0x6B, 0x80};

        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur: Réinitialisation du MPU6050");
                return -1;
        }
        delay(200);
        printf("Réinitialisation du MPU6050\n" );
        return 0;
}

static int config(int *i2c_bus){

        uint8_t buf[2];

        buf[0] = 0x6B;
        buf[1] = 0x00;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur : Reglage de l'alimentation");
                 return -1;
        }
        delay(200);
        printf("Reglage de l'alimentation\n" );

        buf[0] = 0x1A;
        buf[1] = 0x05;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur : Reglage de la synchronisation");
                 return -1;
        }
        delay(200);
        printf("Reglage de la synchronisation\n" );

        buf[0] = 0x1B;
        buf[1] = 0x08;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur : Reglage du Gyroscope");
                return -1;
        }
        delay(200);
        printf("Reglage du Gyroscope\n" );

        buf[0] = 0x1C;
        buf[1] = 0x00;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur : Reglage de l'accélérometre");
                return -1;
        }
        delay(200);
        printf("Reglage de l'accélérometre\n" );

        buf[0] = 0x19;
        buf[1] = 0x07;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur : Reglage du sample rate diviser");
                return -1;
        }
        delay(200);
        printf("Reglage du sample rate diviser\n" );

        buf[0] = 0x38;
        buf[1] = 0x01;
        if(write(*i2c_bus, buf, 2) != 2){
                perror("Erreur : Activation des data");
                return -1;
        }
        delay(200);
        printf("Activation des data\n" );

        return 0;
}

static int mpu_read_raw(int *i2c_bus, int16_t accel[3], int16_t *gyro, int16_t *temp){

        uint8_t data, buffer[6];

        data = 0x3B;
        if(write(*i2c_bus,&data, 1) != 1){
                perror("Error write for accel reading\n");
                return -1;
        }
        delay(1);
        if(read(*i2c_bus, buffer, 6) !=6){
                perror("Error to read accel\n");
                return -1;
        }
        delay(30);
        for (int i = 0; i < 3; i++) {
                accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
        }

        data = 0x41;
        if(write(*i2c_bus,&data, 1) != 1){
                perror("Error write for accel reading\n");
                return -1;
        }
        delay(1);
        if(read(*i2c_bus, buffer, 2) !=2){
                perror("Error to read accel\n");
                return -1;
        }
        delay(30);
        *temp = buffer[0] << 8 | buffer[1];

        data = 0x47;
        if(write(*i2c_bus,&data, 1) != 1){
                perror("Error write for accel reading\n");
                return -1;
        }
        delay(1);
        if(read(*i2c_bus, buffer, 2) !=2){
                perror("Error to read accel\n");
                return -1;
        }
        delay(30);
        *gyro = (buffer[0] << 8 | buffer[1]);

        return 1;
}

static int verif(int *i2c_bus){

        uint8_t buf;

        buf =0x75;
        if(write(*i2c_bus, &buf,1) !=1){
                perror("Error writing on the bus");
                return -1;
        }

        if(read(*i2c_bus,&buf,1)!=1){
                perror("Error reading on the bus");
                return -1;
        }

        if(buf != 104){
                perror("Erreur de lecture/ecriture de registre\n");
                return -1;
        }
        printf("Lecture/ecriture validé\n");
        return 1;

}

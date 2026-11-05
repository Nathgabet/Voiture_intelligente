#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0 //=i2c_default = i2c_inst_t *i2c
#define I2C_SDA 8
#define I2C_SCL 9

// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart1
#define BAUD_RATE 115200

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 4
#define UART_RX_PIN 5

void convet16_to8(uint16_t data, uint8_t *MSB_LSB);
int reset(uint8_t mpu_addr);
int config(uint8_t mpu_addr);
int verif(uint8_t mpu_addr);
int mpu_read_raw(uint8_t mpu_addr, int16_t accel[3], int16_t *gyro, int16_t *temp);

int main()
{
    stdio_init_all();

    uint8_t mpu_addr = 0x68, buf[2];
    int16_t accel[3]= {10}, gyro, temp;

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    

    uart_set_hw_flow(UART_ID, false, false);
        uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
        uart_set_fifo_enabled(UART_ID, false);
    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART
    
    // Send out a string, with CR/LF conversions
    
        reset( mpu_addr);
        config( mpu_addr);
        verif( mpu_addr);


    while (true) {
        printf("Hello, world!\n");

        // mpu_read_raw( mpu_addr,  accel,  &gyro, &temp);
        temp = 4154;

        // printf("- temp: %5.2f\n", (((float)temp) / 340.0) + 36.53);

        convet16_to8(temp, buf);

        uart_write_blocking(UART_ID, buf, 2);

        // uart_putc_raw(UART_ID, 'A');
        // uart_puts(UART_ID, " Hello, UART!\n");


        sleep_ms(100);
    }
}


void convet16_to8(uint16_t data, uint8_t *MSB_LSB){
        
	MSB_LSB[1] = (uint8_t)(data & 0x00FF);
	MSB_LSB[0] = (uint8_t)(data >>8);
        
}

int reset(uint8_t mpu_addr){
        
        uint8_t buf[2] = {0x6B, 0x80};
        //(i2c_write_blocking(*i2c_bus, buf, 2)
        if(i2c_write_blocking(I2C_PORT, mpu_addr, buf, 2, false) != 2){ 
                perror("Erreur: Réinitialisation du MPU6050");
                return -1;
        }
        sleep_ms(200);
        printf("Réinitialisation du MPU6050\n" );
        return 0;
}

int config(uint8_t mpu_addr){

        uint8_t buf[2];

        buf[0] = 0x6B;
        buf[1] = 0x00;
        if(i2c_write_blocking(I2C_PORT, mpu_addr, buf, 2, false) != 2){
                perror("Erreur: Reglage de l'alimentation");
                return -1;
        }
        sleep_ms(200);
        printf("Reglage de l'alimentation\n" );

        buf[0] = 0x1A;
        buf[1] = 0x05;
        if(i2c_write_blocking(I2C_PORT, mpu_addr, buf, 2, false) != 2){
                perror("Erreur: Reglage de la synchronisation");
                 return -1;
        }
        sleep_ms(200);
        printf("Reglage de la synchronisation\n" );

        buf[0] = 0x1B;
        buf[1] = 0x08;
        if(i2c_write_blocking(I2C_PORT, mpu_addr, buf, 2, false) != 2){
                perror("Erreur: Reglage du Gyroscope");
                return -1;
        }
        sleep_ms(200);
        printf("Reglage du Gyroscope\n" );

        buf[0] = 0x1C;
        buf[1] = 0x00;
        if(i2c_write_blocking(I2C_PORT, mpu_addr, buf, 2, false) != 2){
                perror("Erreur: Reglage de l'accélérometre");
                return -1;
        }
        sleep_ms(200);
        printf("Reglage de l'accélérometre\n" );

        buf[0] = 0x19;
        buf[1] = 0x07;
        if(i2c_write_blocking(I2C_PORT, mpu_addr, buf, 2, false) != 2){
                perror("Erreur: Reglage du sample rate diviser");
                return -1;
        }
        sleep_ms(200);
        printf("Reglage du sample rate diviser\n" );

        buf[0] = 0x38;
        buf[1] = 0x01;
        if(i2c_write_blocking(I2C_PORT, mpu_addr, buf, 2, false) != 2){
                perror("Erreur: Activation des datas");
                return -1;
        }
        sleep_ms(200);
        printf("Activation des datas\n" );

        return 0;
}

int verif(uint8_t mpu_addr){

        uint8_t buf;

        buf =0x75;
        if(i2c_write_blocking(I2C_PORT, mpu_addr, &buf, 1, false) !=1){
                perror("Erreur: Ecrire sur le bus");
                return -1;
        }

        if(i2c_read_blocking(i2c_default, mpu_addr, &buf, 1, false)!=1){
                perror("Erreur: Lire sur le bus");
                return -1;
        }
	sleep_ms(1);
        if(buf != 104){
                perror("Erreur: de lecture/ecriture de registre\n");
                return -1;
        }
        printf("Lecture/ecriture validé\n");
        return 1;

}

int mpu_read_raw(uint8_t mpu_addr, int16_t accel[3], int16_t *gyro, int16_t *temp){

        uint8_t data, buffer[6];

        data = 0x3B;
        if(i2c_write_blocking(I2C_PORT, mpu_addr, &data, 1, true) != 1){
                perror("Erreur: Ecriture d'addr pour la lecture de l'accel\n");
                return -1;
        }
        sleep_ms(1);
        if(i2c_read_blocking(i2c_default, mpu_addr, buffer, 6, false) !=6){
                perror("Erreur: Lecture accel\n");
                return -1;
        }
        sleep_ms(5);
        for (int i = 0; i < 3; i++) {
                accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
        }

        data = 0x41;
        if(i2c_write_blocking(I2C_PORT, mpu_addr, &data, 1, true) != 1){
                perror("Erreur: Ecriture d'addr pour la lecture de l'accel\n");
                return -1;
        }
        sleep_ms(1);
        if(i2c_read_blocking(i2c_default, mpu_addr, buffer, 2, false) !=2){
                perror("Erreur: Lecture temp\n");
                return -1;
        }
        sleep_ms(5);
        *temp = buffer[0] << 8 | buffer[1];

        data = 0x43;
        if(i2c_write_blocking(I2C_PORT, mpu_addr, &data, 1, true) != 1){
                perror("Erreur:Ecriture d'addr pour la lecture de l'accel\n");
                return -1;
        }
        sleep_ms(1);
        if(i2c_read_blocking(i2c_default, mpu_addr, buffer, 2, false) !=2){
                perror("Erreur: Lecture gyro\n");
                return -1;
        }
        sleep_ms(5);
        *gyro = (buffer[0] << 8 | buffer[1]);

        return 1;
}

uint8_t checksum(uint8_t *packet, int packet_len){

	uint8_t checksum =0;

	for (int i = 0; i < packet_len; i++) {
        checksum ^= packet[i];
    }
    
	return checksum;
}

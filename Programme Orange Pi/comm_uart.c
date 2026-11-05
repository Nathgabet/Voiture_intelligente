#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <poll.h>

#define TIMEOUT 1000 
#define DATA_LEN 2

//gcc -Wall -Wextra comm_uart.c -o comm_uart

void convet16_to8(uint16_t data, uint8_t *MSB_LSB);

int Uart_Connect (char pathlidar[]) {
	/*Function to set the uart communication
	*/
	struct termios options;
	uint8_t fd;
	
	fd = open(pathlidar, O_RDWR | O_NOCTTY | O_SYNC);
	if(fd == 255){
		printf("Error Open UART bus\n");
		return -1;
	}
	printf("Uart open\n");
	
    if(tcgetattr(fd, &options) < 0){
  		printf("Err tcgetattr\n");
  		close(fd);
		return -1;
 	}

    cfsetospeed(&options, 115200);
    cfsetispeed(&options, 115200);
    options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;
    options.c_lflag = 0;
    options.c_oflag = 0;
    options.c_cc[VMIN] = 0; 
	options.c_cc[VTIME] = 10;
    if(tcsetattr(fd, TCSANOW, &options) < 0 ){
		printf("Setting serial parameters'Failed to connect to the sensor \n");
		close(fd);
		return -1;
	}
	printf("Lidar Connect\n");
	return fd;
}

uint8_t read_raw(int fd, uint8_t *buffer, int length) {
    /*
 		Function to received the raw data of the lidar
 	*/
	struct pollfd pfd;
	int bytes_read = 0, poll_res, len_read;

    pfd.fd = fd;
    pfd.events = POLLIN;
    
    while (bytes_read < length) {
        poll_res = poll(&pfd, 1, TIMEOUT);

        if (poll_res <= 0) {
            return 1;
        }

        if (pfd.revents & POLLIN) {
            len_read = read(fd, buffer + bytes_read, length - bytes_read);
            if (len_read <= 0) {
        		return 1;
            }
            bytes_read += len_read;
        }
    }
    return 0;
}

int main(){

	uint16_t temperature ;
	uint8_t data[DATA_LEN];
	int fd;

	fd = Uart_Connect("/dev/ttyAMA1");

	read_raw(fd, data, DATA_LEN);
	temperature =  (data[0] <<8 | data[1] );

	for(int i =0; i<DATA_LEN;i++){
		printf("data [%d]: %d %X\n", i, data[i],  data[i]);
	}
	printf("temperature is : %f raw is : %d\n",(((float)temperature) / 340.0) + 36.53, temperature);

	return 0;
}



void convet16_to8(uint16_t data, uint8_t *MSB_LSB){
        
	MSB_LSB[1] = (uint8_t)(data & 0x00FF);
	MSB_LSB[0] = (uint8_t)(data >>8);
        
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
//#include <sys/ioctl.h>
//#include "lxlib.h"

//gcc -Wall testLX.c -o testLX

int main(void){

	int fd, len, lendata;

	struct termios options; //seriel port setting
	uint8_t data[2] = {0};
	uint8_t value[32] = {0};

	fd = open("/dev/ttyAMA0", O_RDWR );
	if(fd < 0){
		perror("Error Open UART bus");
		return -1;
	}
	printf("Uart open\n");
	//Set up serial port

	if(tcgetattr(fd, &options) < 0){
  		printf("Err tcgetattr\n");
  		close(fd);
 	}

	options.c_cflag = B460800 | CS8 | CREAD | HUPCL |CLOCAL; //baudrade, data size
	options.c_iflag = IXON;
	options.c_oflag = 0;
	options.c_lflag = 0;

	//apply the settings
	tcflush(fd, TCIFLUSH);
	if(tcsetattr(fd, TCSANOW, &options) < 0 ){
		perror("Setting serial parameters \n");
		close(fd);
		return -1;
	}
	printf("serial set\n");
	len = 2;

	data[0] = 0xA5;
    data[1] = 0x50;

	len = write(fd, data, len);
	printf(" %x %x\n", data[0], data[1]);
    printf("Send Request lidar info %x len:%ld \n",data[1], sizeof(len));

/*	scanf("%d", &data[1]);
	printf("%d \n", data[1]);
//	usleep(10);
*/

	lendata = 0;

	while(lendata <4){

		lendata= read(fd, value, 26);
		//usleep(5);
	}

	printf("Received %d bytes \n", lendata);
	for(int i = 0; i< lendata; i++){
		printf(" value[%d] : received : %d  %02x\n",i, value[i], value[i]);
	}

	usleep(100*100);

	close(fd);
	usleep(100*100);

	return 0;
}

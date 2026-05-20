#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h> 
#include <poll.h>

#define KRED  "\x1B[31m"
#define KGRN   "\x1B[32m"
#define KRESET "\x1B[0m"
#define KYEL   "\x1B[33m"

#define TIMEOUT 1000

#define SYNC_BYTE 0xA5
#define SYNC_BYTE2 0x5A

#define DESCRIPTOR_LEN 7
#define INFO_LEN 20
#define EXPRESS_SCAN_LEN 84
#define HEALTH_LEN 3
#define MEASURE_LEN 5
#define SAMPLERATE_LEN 4

#define GET_INFO_BYTE  0x50
#define GET_HEALTH_BYTE  0x52
#define GET_SAMPLERATE 0x59

#define MAX_MOTOR_PWM  1023
#define DEFAULT_MOTOR_PWM  660
#define MOTOR_SPEED_CTRL 0xA8
#define SET_PWM_BYTE 0xF0 //register related to the 
#define STOP_BYTE 0x25
#define RESET 0x40
#define SCAN_BYTE 0x20
#define EXPRESS_SCAN 0x82

struct lidar{
	uint8_t fd;
    uint8_t motor_speed ;
    uint8_t motor_running ;
    uint8_t connect;
	uint8_t scanning;
};

uint8_t _checksum(uint8_t *packet, int packet_len){

	uint8_t checksum =0;

	for (int i = 0; i < packet_len; i++) {
        checksum ^= packet[i];
    }
    
	return checksum;
}

float _process_scan(uint8_t *raw){
	/*
	Function that extract the diffrents value of the data
	*/
	uint8_t new_scan = raw[0] &  0b00000001;
	uint8_t invert_new_scan = (raw[0] >> 1) &  0b00000001;
	uint8_t quality = raw[0] >> 2 ;
	uint8_t check_bit = raw[1] & 0b00000001;

	if ((new_scan == invert_new_scan) || (check_bit != 1)){
		//printf(KYEL" New flag mismatch \n"KRESET);
		return -1;
	}
	else{
		float angle = ((raw[1] >> 1) | ((uint16_t)raw[2] << 7)) / 64. ;
		float distance = ((uint16_t)(raw[4] <<8) | raw[3]) / 4. ;
		
		printf(KGRN"angle: %.2f distance: %.2f quality: %d\n"KRESET, angle, distance, quality);
		fflush(stdout);
	}

	return 0;	
}

int LidarConnect (char pathlidar[]) {
	/*Function to set the uart communication
	//Also try with those parameters for the ildar setting 
	*/

	struct termios options;
	uint8_t fd;

	fd = open(pathlidar, O_RDWR | O_NOCTTY | O_SYNC);
	if(fd == 255){
		perror("Error Open UART bus");
		return -1;
	}
	printf("Uart open\n");
	
    if(tcgetattr(fd, &options) < 0){
  		printf(KRED"Err tcgetattr\n"KRESET);
  		close(fd);
		return -1;
 	}

    cfsetospeed(&options, B460800);
    cfsetispeed(&options, B460800);
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
	printf("serial set\n");

	return fd;

}

int _read_descriptor(uint8_t *raw_data){
	/*
	Check if it's the right lidar and erase the file descriptor of the response
	*/
	uint32_t response_length;
	uint8_t send_mode, flag =1;

	if(sizeof(raw_data) < DESCRIPTOR_LEN){
		perror("Error with the lens of the gethealth file descriptor\n");
	}
	if( (raw_data[0]!= SYNC_BYTE) && (raw_data[1] != SYNC_BYTE2)){
		perror("No sync byte");
		flag = -1;
	}
	else{
		printf("Request Accepted\n");
	}
	response_length = ((uint32_t)raw_data[2]<<24) | ((uint32_t)raw_data[3]<<16) | ((uint32_t)raw_data[4]<<8) | ((uint32_t)raw_data[5] & 0b11111100); 
	printf("response_length : %x \t", response_length);
	send_mode = raw_data[5] & 0b00000011;
	printf("send_mode : %x \t", send_mode);
	printf("data type : %x \n", raw_data[6]);
	
	return flag;
}

void _send_cmd(uint8_t fd, uint8_t cmd){
	/*
	Function to send data to the lidar
	*/
	uint8_t package[2] = {0xA5, cmd};
	write(fd, package, 2);
	
}

uint8_t _read_raw(uint8_t fd, uint8_t *buffer, int length) {
    /*
 		Function to received the raw data of the lidar
 	*/
	struct pollfd pfd;
	int bytes_read = 0, poll_res, len_read;

    pfd.fd = fd;
    pfd.events = POLLIN;
    
    while (bytes_read < length) {
        // Wait for data to be available on the file descriptor
        poll_res = poll(&pfd, 1, TIMEOUT);

        if (poll_res < 0) {
            perror("Poll error");
            return 1;
        } else if (poll_res == 0) {
			printf(KRED "TimeOut Error\t" KRESET);
            return 2; 			
        }

        if (pfd.revents & POLLIN) {
            len_read = read(fd, buffer + bytes_read, length - bytes_read);
            if (len_read < 0) {
                perror("Read error");
                return 1;
            } else if (len_read == 0) {
                // Unexpected end of file/disconnection
                return 1;
            }
            bytes_read += len_read;
        }
    }

    return 0;
}

int LidarDeconnect (uint8_t fd){
	/*
	Function that deconnect the lidar 
	*/
	printf("Lidar deconnect\n");
    close(fd);

    return 0;
}

int motor_speed(uint8_t fd , uint16_t rpm){
	/*
	Function that check the value of the motor speed before send the command
	*/
	//self._set_pwm(self._motor_speed)
    //self.motor_running = True

    uint8_t packet[6];

    packet[0] = 0xA5; 
    packet[1] = MOTOR_SPEED_CTRL;
    packet[2] = 2; //Payload size, RPM value on 2 byte
    packet[3] = (uint8_t)(rpm & 0xFF);
    packet[4] = (uint8_t)((rpm >> 8) & 0xFF);
  	packet[5] = _checksum(packet,5);

    write(fd, packet, 6);
    printf("Motor speed request sent: %d RPM\n", rpm);
	usleep(900000);

	return 0;
}

int start_motor(uint8_t fd ){
	/*
	Function that start the motor
	*/
	printf("Star motor\n");
	if (motor_speed(fd, DEFAULT_MOTOR_PWM) <0){
		perror("Motor didn't Start");
		return -1;
	}

	return 0;
}

int stop_motor(uint8_t fd ){
	/*
	Function that stop the motor
	*/
	printf("Stop motor\n");
	if (motor_speed(fd, 0) <0){
		perror("Motor didn't Start");
		return -1;
	}

	return 0;
}

void _get_samplerate(uint8_t fd){

	uint8_t raw[7];
	int flag;
	printf("Get samplerate \n");

	_send_cmd(fd, GET_SAMPLERATE);
	_read_raw(fd, raw, DESCRIPTOR_LEN);
	// _read_descriptor(raw);	

	if((flag =_read_raw(fd, raw, SAMPLERATE_LEN)) == 2 ){
		printf(KRED"Timeout on _get_samplerate\n"KRESET);
		return;
	}
	else{
		printf("Tstandard for Scan: %dus\n", ((raw[1]<<8) |(raw[0])));
		printf("Texpress for Express Scan : %dus\n", ((raw[3]<<8) |(raw[2])));
	}
}

int iter_measurement (uint8_t fd){
	
	uint8_t valeurs[5], unsync;
	int errror_count= 0;

	read(fd, valeurs, MEASURE_LEN) ;
	if(_process_scan(valeurs) <0){
		errror_count++;
		read(fd, &unsync,1);
		return errror_count;
	}

	return errror_count;
}

void reset(uint8_t fd){
	uint8_t flag[5];

	_send_cmd(fd, RESET);
	usleep(1000 *1000);
	start_motor(fd);
	read(fd,flag, 5);
	_read_descriptor(flag);
}

uint16_t gethealth(uint8_t fd){
	/*Get device health state

        Returns
        -------
        status : uint8_t
            0 = 'Good', 1 = 'Warning' or 2 = 'Error' statuses
        error_code : uint8_t
            The related error code that caused a warning/error.
    */
    uint8_t raw[10];
	uint16_t error_code;
    	
	printf("Gethealth\n");

	_send_cmd(fd, GET_HEALTH_BYTE);
	_read_raw(fd, raw, DESCRIPTOR_LEN);
	_read_descriptor(raw);
	
	if (_read_raw(fd, raw, HEALTH_LEN) > 1){
		printf(KRED"Timeout on gethealth\n"KRESET);
		return -1;
	}
	printf("health statue : %x\t", raw[0]);
	error_code = (raw[2] <<8) | raw[1];

	if(raw[0] == 2){
		printf(KRED"error_code: %d\n"KRESET, error_code);
		return -1;
	}
	else if(raw[0] ==1){
		printf(KYEL"Warning: %d\n"KRESET, error_code);
	}
	else{
		printf("Status GOOD\n");
	}
	return 0;
}

void getinfo(uint8_t fd){
	/*
		Fonction pour recuperer les infos du Lidar (Model, Hardaware, Firmware, Serial Number)
	Try this way, chak the file descriptor and after read the data of interrest
	*/
	uint8_t raw[20];
	printf("Get Infos \n");
	
	_send_cmd(fd, GET_INFO_BYTE);
	_read_raw(fd, raw, DESCRIPTOR_LEN);
	_read_descriptor(raw);

	_read_raw(fd, raw, INFO_LEN);
	
	printf(" - model : %x \n", raw[0]>>3);
	printf(" - firmware : %x.%x \n", raw[2], raw[1]);
	printf(" - hardware : %x \n", raw[3]);
	printf(" - serial_number : ");
	for(int i =4; i < INFO_LEN; i++){
		printf("%x",raw[i]);
	}
	printf("\n");
	
}

void stop(uint8_t fd){
	printf("Stop function\n");
	_send_cmd(fd, STOP_BYTE);
	usleep(20*1000);
	
}

int startreadmesurement(uint8_t fd){

	uint8_t descriptor[7];

	start_motor(fd);
	_send_cmd(fd, SCAN_BYTE);
	read(fd, descriptor, DESCRIPTOR_LEN );
	
	if(_read_descriptor(descriptor) <0){
		printf(KRED"SCAN didn't start\n"KRESET);
		return -1;		
	}
	else{
		printf("Start SCAN Measurement\n");
	}
	usleep(500000);
	printf("Measurement Available\n");
	tcflush(fd,TCIFLUSH);

	return 0;
}

void express_scan(uint8_t fd){

	uint8_t request[8], response[84];

	request[0] = 0x82;
	request[1] = 5;
	for(int i = 2; i<7;i++){
		request[i] = 0x00;
	}
	request[8] = 22;

	_send_cmd(fd, *request);
	read(fd, response, EXPRESS_SCAN_LEN );
	read(fd, response, EXPRESS_SCAN_LEN );
	
	for(int i = 0; i<85;i++ ){

		printf("value %d response : %d %x \n", i,response[i], response[i] );
	}


}

int main (void){

	struct lidar new_lidar;
	struct timeval start_time, stop_time;
	long int error_count = 0, error_count_flag =0, i=0;
	
	if((new_lidar.fd = LidarConnect("/dev/ttyUSB0")) ==255){
		printf(KRED"Lidar connexion failed \n"KRESET);
		return -1;
	}
	
	getinfo(new_lidar.fd);
	gethealth(new_lidar.fd);
	_get_samplerate(new_lidar.fd);
	startreadmesurement(new_lidar.fd);
	
	//express_scan(new_lidar.fd);

	gettimeofday(&start_time, NULL);
	do{
		error_count +=iter_measurement(new_lidar.fd); 
		gettimeofday(&stop_time, NULL);
		if (error_count_flag ==	error_count){
			printf("measurement %ld ", i+1);
		}
		i++; error_count_flag =	error_count;
	}while(((stop_time.tv_sec - start_time.tv_sec) < 10));

	printf("valid measurment : %.2f%%\n",  (((float)i-(float)error_count)/(float)i)*100.0);
	printf("time of measurment = %ld s\n", (stop_time.tv_sec - start_time.tv_sec));

	stop(new_lidar.fd);
	LidarDeconnect(new_lidar.fd);

    return 0;
}
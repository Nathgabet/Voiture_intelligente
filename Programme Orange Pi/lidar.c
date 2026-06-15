#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h> 
#include <pthread.h>
#include <poll.h>
#include "../include/lidar.h"

lidar global_new_lidar;

uint8_t checksum(uint8_t *packet, int packet_len){

	uint8_t checksum =0;

	for (int i = 0; i < packet_len; i++) {
        checksum ^= packet[i];
    }
    
	return checksum;
}

float process_scan(uint8_t *raw){
	/*
	Function that extract the diffrents value of the data
	*/
	uint8_t new_scan = raw[0] &  0b00000001;
	uint8_t invert_new_scan = (raw[0] >> 1) &  0b00000001;
	global_new_lidar.quality = raw[0] >> 2 ;
	uint8_t check_bit = raw[1] & 0b00000001;
	float temp_distance,temp_angle;

	if ((new_scan == invert_new_scan) || (check_bit != 1)){
		return -1;
	}
	else{
		
		temp_angle = ((raw[1] >> 1) | ((uint16_t)raw[2] << 7)) / 64. ;
		if(temp_angle < 360 && temp_angle > 0){ 
			global_new_lidar.angle  = temp_angle;
		}
		
		temp_distance = ((uint16_t)(raw[4] <<8) | raw[3]) / 4. ;
		if(temp_distance < 12000 && temp_distance > 0){ 
			global_new_lidar.distance  = temp_distance;
		}
		fflush(stdout);
		return 0;
	}
}

int LidarConnect (char pathlidar[]) {
	/*Function to set the uart communication
	*/
	struct termios options;
	
	global_new_lidar.fd = open(pathlidar, O_RDWR | O_NOCTTY | O_SYNC);
	if(global_new_lidar.fd == 255){
		printf("Error Open UART bus\n");
		return -1;
	}
	printf("Uart open\n");
	
    if(tcgetattr(global_new_lidar.fd, &options) < 0){
  		printf("Err tcgetattr\n");
  		close(global_new_lidar.fd);
		return -1;
 	}

    cfsetospeed(&options, B460800);
    cfsetispeed(&options, B460800);
    options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;
    options.c_lflag = 0;
    options.c_oflag = 0;
    options.c_cc[VMIN] = 0; 
	options.c_cc[VTIME] = 10;
    if(tcsetattr(global_new_lidar.fd, TCSANOW, &options) < 0 ){
		printf("Setting serial parameters'Failed to connect to the sensor \n");
		close(global_new_lidar.fd);
		return -1;
	}
	printf("Lidar Connect\n");
	return global_new_lidar.fd;
}

int _read_descriptor(uint8_t *raw_data){
	/*
	Check if it's the right lidar and erase the file descriptor of the response
	*/
	uint32_t response_length;
	uint8_t send_mode;

	if(sizeof(raw_data) < DESCRIPTOR_LEN){
		printf("Error with the lens of the gethealth file descriptor\n");
	}
	if( (raw_data[0]!= SYNC_BYTE) && (raw_data[1] != SYNC_BYTE2)){
		printf("No sync byte\n");
		return -1;
	}
	else{
		printf("Request Accepted\n");
	}
	response_length = ((uint32_t)raw_data[2]<<24) | ((uint32_t)raw_data[3]<<16) | ((uint32_t)raw_data[4]<<8) | ((uint32_t)raw_data[5] & 0b11111100); 
	printf( "response_length : %x \t", response_length);
	send_mode = raw_data[5] & 0b00000011;
	printf("send_mode : %x \t", send_mode);
	printf("data type : %x \n", raw_data[6]);
	
	return 1;
}

void _send_cmd(uint8_t cmd){
	/*
	Function to send data to the lidar
	*/
	uint8_t package[2] = {0xA5, cmd};
	write(global_new_lidar.fd, package, 2);
	
}

uint8_t _read_raw(uint8_t *buffer, int length) {
    /*
 		Function to received the raw data of the lidar
 	*/
	struct pollfd pfd;
	int bytes_read = 0, poll_res, len_read;

    pfd.fd = global_new_lidar.fd;
    pfd.events = POLLIN;
    
    while (bytes_read < length) {
        poll_res = poll(&pfd, 1, TIMEOUT);

        if (poll_res <= 0) {
            return 1;
        }

        if (pfd.revents & POLLIN) {
            len_read = read(global_new_lidar.fd, buffer + bytes_read, length - bytes_read);
            if (len_read <= 0) {
        		return 1;
            }
            bytes_read += len_read;
        }
    }
    return 0;
}

int LidarDeconnect (){
	/*
	Function that deconnect the lidar 
	*/
    close(global_new_lidar.fd);
	printf("Lidar disconnected \n");
    return 0;
}

int motor_speed(uint16_t rpm){
	/*
	Function that check the value of the motor speed before send the command
	*/
    uint8_t packet[6];

    packet[0] = 0xA5; 
    packet[1] = MOTOR_SPEED_CTRL;
    packet[2] = 2; //Payload size, RPM value on 2 byte
    packet[3] = (uint8_t)(rpm & 0xFF);
    packet[4] = (uint8_t)((rpm >> 8) & 0xFF);
  	packet[5] = checksum(packet,5);

    write(global_new_lidar.fd, packet, 6);
	usleep(900000);

	return 0;
}

int start_motor(){
	/*
	Function that start the motor
	*/
	if (motor_speed(DEFAULT_MOTOR_PWM) <0){
		perror("Motor didn't Start");
		return -1;
	}

	return 0;
}

int stop_motor(){
	/*
	Function that stop the motor
	*/
	if (motor_speed(0) <0){
		return -1;
	}

	return 0;
}

void get_samplerate(){

	uint8_t raw[7];
	int flag;
	printf("Get samplerate \n");

	_send_cmd(GET_SAMPLERATE);
	_read_raw(raw, DESCRIPTOR_LEN);
	_read_descriptor(raw);	

	if((flag =_read_raw(raw, SAMPLERATE_LEN)) == 2 ){
		printf("Timeout on get_samplerate\n");
		return;
	}
	else{
		printf("Tstandard for Scan: %dus\n", ((raw[1]<<8) |(raw[0])));
		printf("Texpress for Express Scan : %dus\n", ((raw[3]<<8) |(raw[2])));
	}
}

void *loop_measurement(void *arg){
	
	free(arg);
	uint8_t valeurs[5], unsync;

	read(global_new_lidar.fd,valeurs, MEASURE_LEN) ;
	if(process_scan(valeurs) <0){
		global_new_lidar.error_count++;
		read(global_new_lidar.fd,&unsync,1);
	}
	pthread_exit(NULL);
}

void iter_measurement(){
	
	uint8_t valeurs[5], unsync;

	read(global_new_lidar.fd,valeurs, MEASURE_LEN) ;
	if(process_scan(valeurs) <0){
		global_new_lidar.error_count++;
		read(global_new_lidar.fd,&unsync,1);
	}
}

void reset(){
	uint8_t flag[5];

	_send_cmd(RESET);
	usleep(1000 *1000);
	start_motor();
	read(global_new_lidar.fd,flag, 5);
	_read_descriptor(flag);
}

uint16_t gethealth(){
	/*Get device health state

        Returns
        -------
        status : uint8_t
            0 = 'Good', 1 = 'Warning' or 2 = 'Error' statuses
        error_code : uint8_t
            The related error code that caused a warning/error.
    */
    uint8_t raw[10], error_code;

	_send_cmd(GET_HEALTH_BYTE);
	_read_raw(raw, DESCRIPTOR_LEN);
	_read_descriptor(raw);
	
	error_code = (raw[2] <<8) | raw[1] ;

	if (_read_raw(raw, HEALTH_LEN) > 1){
		printf( "Timeout on gethealth\n");
		LidarDeconnect(); 
		return -1;
	}
	printf("health statue : %x\t", error_code);

	if(raw[0] == 2){
		printf("error_code: %d\n", error_code);
		LidarDeconnect(); 
		return -1;
	}
	else if(raw[0] ==1){
		printf("Warning: %d\n", error_code);
	}
	else{
		printf("Status GOOD\n");
	}
	return 0;
}

void getinfo(){
	/*
		Fonction pour recuperer les infos du Lidar (Model, Hardaware, Firmware, Serial Number)
	Try this way, chak the file descriptor and after read the data of interrest
	*/
	uint8_t raw[20];
	printf("Get Infos \n");
	
	_send_cmd(GET_INFO_BYTE);
	_read_raw(raw, DESCRIPTOR_LEN);
	_read_descriptor(raw);

	_read_raw(raw, INFO_LEN);
	
	printf(" - model : %x \n", raw[0]>>3);
	printf(" - firmware : %x.%x \n", raw[2], raw[1]);
	printf(" - hardware : %x \n", raw[3]);
	printf(" - serial_number : ");
	for(int i =4; i < INFO_LEN; i++){
		printf("%x",raw[i]);
	}
	printf("\n");
	
}

void stop_everything(){
	_send_cmd(STOP_BYTE);
	usleep(20*1000);
	printf("lidar stopted\n");
	pthread_cancel(global_new_lidar.handle_thread);
	printf("thread quit\n");
	LidarDeconnect();
}

int startreadmesurement(){

	uint8_t descriptor[7];

	start_motor();
	_send_cmd(SCAN_BYTE);
	read(global_new_lidar.fd, descriptor, DESCRIPTOR_LEN );
	
	if(_read_descriptor(descriptor) <0){
		printf("SCAN didn't start\n");
		return -1;		
	}
	else{
		printf("Start SCAN Measurement\n");
	}
	usleep(500000);
	printf("Measurement Available\n");
	tcflush(global_new_lidar.fd,TCIFLUSH);

	return 0;
}

int LidarInit(){

	global_new_lidar.errorcode	= 0;
	global_new_lidar.error_count = 0;

	if (LidarConnect("/dev/ttyUSB0") <=0){
		return -1;
	}
	getinfo(global_new_lidar.fd);
	gethealth(global_new_lidar.fd);
	get_samplerate(global_new_lidar.fd);
	if(startreadmesurement(global_new_lidar.fd)){
		perror("start measurement");
		LidarDeconnect();
		return -1;
	}
	else{
		if(pthread_create(&global_new_lidar.handle_thread,NULL,loop_measurement, NULL)==0){
			printf("Thread created\n");
		}
		else{
			printf("pthread_create failed\n");
		}
	}	
	printf("LidarInit success\n");
	return 0;
}

void report(double number_iteration, long int timepast){

	printf("valid measurment : %.2f%%\n",  (((float)number_iteration-(float)global_new_lidar.error_count)/(float)number_iteration)*100.0);
	printf("time of measurment = %ld s\n", timepast);

}

int main (void){

	struct timeval start_time, stop_time;
	long int i=0;

	if(LidarInit("/dev/ttyUSB0") <0){
		return -1;
		LidarDeconnect(); 
	}

	gettimeofday(&start_time, NULL);
	do{
		iter_measurement(); 
		printf("angle %.2lf, distance %.2lf\n", global_new_lidar.angle, global_new_lidar.distance);
		gettimeofday(&stop_time, NULL);
		i++;
	}while(((stop_time.tv_sec - start_time.tv_sec) < 10));
	report(i, (stop_time.tv_sec - start_time.tv_sec) );
	printf("report success\n");
	
	stop_everything();                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
	printf("out success\n");
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h> 

// gcc -Wall clidar.c -o clidar
//ctrl / pour dé/commenter
#define KRED  "\x1B[31m"
#define KGRN   "\x1B[32m"
#define KRESET "\x1B[0m"
#define KYEL   "\x1B[33m"

#define SYNC_BYTE 0xA5
#define SYNC_BYTE2 0x5A
#define GET_INFO_BYTE  0x50
#define GET_HEALTH_BYTE  0x52
#define DESCRIPTOR_LEN 7
#define MAX_MOTOR_PWM  1023
#define DEFAULT_MOTOR_PWM  660
#define MOTOR_SPEED_CTRL 0xA8
#define SET_PWM_BYTE 0xF0 //register related to the 
#define STOP_BYTE 0x25
#define INFO_LEN 20
#define EXPRESS_SCAN_LEN 84
#define HEALTH_LEN 3
#define RESET 0x40
#define SCAN_BYTE 0x20
#define EXPRESS_SCAN 0x82

struct lidardata{
	float angle;
	float distance;
	float quality;
};

struct lidarinfo {
	char model[2];
	char firmware[2];
	char hardware[2];
	char serial_number[18];
};

struct lidar{
	uint8_t fd;
    uint8_t motor_speed ;
    uint8_t motor_running ;
    uint8_t connect;
	uint8_t scanning;
	struct lidarinfo info;
	struct lidardata data;
};

float  _process_scan(uint8_t *raw){
	/*
	Function that extract the diffrents value of the data
	*/
	uint8_t new_scan = raw[0] &  0b1;
	uint8_t invert_new_scan = (raw[0] >> 1) &  0b1;
	uint8_t quality = raw[0] >> 2 ;
	
	if (new_scan == invert_new_scan){
		printf(KYEL" New flag mismatch \n"KRESET);
	}
	uint8_t check_bit = raw[1] & 0b1;
	if (check_bit != 1){
		printf(KYEL" check_bit not equal to 1 \n"KRESET);
	}
	float angle = ((raw[1] >> 1) + (raw[2] << 7)) / 64. ;
	float distance = (raw[3] + (raw[4] <<8)) / 4. ;
	
	float data[3] = {angle, distance, quality} ;
	
	return *data;
}

int LidarConnect (char pathlidar[], struct lidar *self) {
	/*Function to set the uart communication
	//Also try with those parameters for the ildar setting 
	*/

	struct termios options;

	(*self).fd = open(pathlidar, O_RDWR | O_NOCTTY | O_SYNC);
	if((*self).fd < 0){
		perror("Error Open UART bus");
		return -1;
	}
	printf("Uart open\n");
	
    if(tcgetattr((*self).fd, &options) < 0){
  		printf("Err tcgetattr\n");
  		close((*self).fd);
 	}

    cfsetospeed(&options, B460800);
    cfsetispeed(&options, B460800);
    options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;
    options.c_lflag = 0;
    options.c_oflag = 0;
    options.c_cc[VMIN] = 0; 
	options.c_cc[VTIME] = 10; // 1s timeout
    if(tcsetattr((*self).fd, TCSANOW, &options) < 0 ){
		perror("Setting serial parameters'Failed to connect to the sensor \n");
		close((*self).fd);
		return -1;
	}
	printf("serial set\n");

	return 0;

}

void _read_descriptor(uint8_t *raw_data){
	/*
	Check if it's the right lidar and erase the file descriptor of the response
	*/
	if( (raw_data[0]!= SYNC_BYTE) && (raw_data[1] != SYNC_BYTE2)){
		perror("No sync byte");
	}
	if (sizeof(*raw_data) >2){
		for(int i =0; i<sizeof(*raw_data)-2; i++){
			raw_data[i] = raw_data[i+7]; //revoir si la reponse ne correspond pas
		}
	}
	else{
		perror("Data to Short");
	}
	
}

void _send_cmd(int fd, uint8_t cmd){
	/*
	Function to send data to the lidar
	*/
	uint8_t package[2] = {0xA5, cmd};
	write(fd, package, 2);
	
}

void _read_raw(int fd, uint8_t *value, int lendata){
	/*
	Function to received the raw data of the lidar
	*/
	uint8_t len = 0;
	uint8_t data[32] ={0};
	
	while(len < 5){ //need to had a watchdog on this measurement
		len = read(fd, data, lendata );
	}
	for(int i =0; i<= lendata; i++){
        value[i] = data[i];
    }
	
}

int LidarDeconnect (int fd){
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

    uint8_t packet[6], checksum;
    
    packet[0] = 0xA5; 
    packet[1] = MOTOR_SPEED_CTRL;
    packet[2] = 2;

    packet[3] = (uint8_t)(rpm & 0xFF);
    packet[4] = (uint8_t)((rpm >> 8) & 0xFF);
    
 
    for (int i = 0; i < 5; i++) {
        checksum ^= packet[i];
    }
    packet[5] = checksum;

    write(fd, packet, 6);
    
    printf("Motor speed request sent: %d RPM\n", rpm);


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

int iter_measurement (struct lidar self){
	
	uint8_t valeurs[5];

	read(self.fd, valeurs, 5) ;

	printf( "valeurs:" );
	for (int i =0; i<5; i++){
		printf(" %x", valeurs[i]);
	}
	printf("\n");

	// float angle = (((uint16_t)valeurs[2] << 7) | (valeurs[1] >> 1)) / 64.0f;
	// float dist = (((uint16_t)valeurs[4] << 8) | valeurs[3]) / 4.0f;          
	// printf("Angle: %.2f deg, Dist: %.2f mm\n", angle, dist);

	// *data = _process_scan(valeurs);
	// printf("process : Angle: %.2f deg, Dist: %.2f mm\n", data[0], data[1]);

	// if(sizeof(valeurs) != 5 ){
	// 	printf(KRED"Invalid packet detected, skipping...\n" KRESET);
	// }

	return 0;
}

void reset(uint8_t fd){

	_send_cmd(fd, RESET);
	usleep(700 *1000);
}

uint8_t gethealth(struct lidar self){
	/*Get device health state

        Returns
        -------
        status : uint8_t
            0 = 'Good', 1 = 'Warning' or 2 = 'Error' statuses
        error_code : uint8_t
            The related error code that caused a warning/error.
    */
    uint8_t raw[7], error_code;
    	
	printf("Gethealth\n");

	_send_cmd(self.fd, GET_HEALTH_BYTE);
	if(sizeof(raw) < DESCRIPTOR_LEN){
		perror("Error with the lens of the gethealth file descriptor\n");
	}
	_read_raw(self.fd, raw, HEALTH_LEN);

	printf("health statue : %d\n", raw[0]);
	error_code = (raw[1] <<8) + raw[2];

	/*//try this way after the send command
	 if (read(fd, desc, 7) == 7 && desc[0] == 0xA5) {
        read(fd, health, 3);
        uint8_t status = health[0]; // 0:Good, 1:Warning, 2:Error 
        if (status == 2) {
            printf("Error detected! Code: %02X%02X\n", health[2], health[1]);
            return false;
        }
        printf("Health Status: %s\n", status == 0 ? "Good" : "Warning");
		
	*/

	return error_code;
}

void getinfo(struct lidar *self){
	/*
		Fonction pour recuperer les infos du Lidar (Model, Hardaware, Firmware, Serial Number)
	Try this way, chak the file descriptor and after read the data of interrest
	*/

	printf("Get Infos \n");

	uint8_t raw[20];
	
	_send_cmd((*self).fd, GET_INFO_BYTE);
	_read_raw((*self).fd, raw, DESCRIPTOR_LEN);
	if(sizeof(raw) < DESCRIPTOR_LEN){
		perror("Error with the lens of the getinfo file descriptor\n");
	}
	_read_raw((*self).fd, raw, INFO_LEN);
	
	printf(" - model : %x \n", raw[0]>>3);
	printf(" - firmware : %x.%x \n", raw[2], raw[1]);
	printf(" - hardware : %x \n", raw[3]);
	printf(" - serial_number : %x", raw[4]);
	for(int i =4; i < sizeof(raw); i++){
		printf("%x",raw[i]);
	}
	printf("\n");
	
}

void  stop(int fd){
	printf("Stop function\n");
	_send_cmd(fd, STOP_BYTE);
	usleep(1*100);
	
}

void startreadmesurement(int fd){

	uint8_t descriptor[7];

	_send_cmd(fd, SCAN_BYTE);

	read(fd, descriptor, DESCRIPTOR_LEN );
	

	if(descriptor[0]== 0xA5 && descriptor[1]== 0x5A ){
		printf("Start SCAN Measurement\n");
	}
	else{
		printf(KRED"SCAN didn't start\n"KRESET);
		// reset(fd);
	}

	printf("descriptor : ");
	for(int i =0; i<7;i++){
			printf(" %x", descriptor[i]);
	}
	printf("\n");

}

void express_scan(int fd){

	uint8_t request[8], response[84];

	request[0] = 0x82;
	request[1] = 0x82;
	for(int i = 2; i<7;i++){
		request[i] = 0x00;
	}
	request[8] = 22;

	_send_cmd(fd, *request);

	read(fd, response, EXPRESS_SCAN_LEN );
	

	for(int i = 0; i<85;i++ ){

		printf("value %d response : %d %x \n", i,response[i], response[i] );
	}


}


int main (void){

	struct lidar new_lidar;
	
	if( LidarConnect("/dev/ttyUSB0", &new_lidar) <0){
		perror(KYEL"Lidar connexion failed "KRESET);
	}

	//reset(new_lidar.fd);
	getinfo(&new_lidar);

	// reset(new_lidar.fd);
	
	// if(gethealth(new_lidar)!=0){
	// 	printf("Error code : %d \n", error_code);
	// 	LidarDeconnect(new_lidar.fd);
	// }
	
	// startreadmesurement(new_lidar.fd);

	// for(int i=0; i<20; i++){
	// 	printf("measurement %d ", i+1);
	// 	iter_measurement(new_lidar); 
		
	// }

	express_scan(new_lidar.fd);

	stop(new_lidar.fd);
	//stop_motor(new_lidar.fd);
	LidarDeconnect(new_lidar.fd);

    return 0;
}










/*
	Information sur l'evitement d'obstacle:
	Méthode réactive: champs des potentiels VFH/VFH+ choix d'orientation sans collision
	Échantillonnage/optimisation: MPC pour drones/voitures
	Objets dynamiques: prédiction à court terme (ex. modèle constant-virage-vitesse) et “marges de temps” (Time-To-Collision, TTC) pour décider ralentir, céder, contourner.
	Génération de trajectoire selon la vitesse + suivies de trajectoire (PID/MPC)
	Watchdog de sécurisation

Points durs et bonnes pratiques

Gérer faux positifs/négatifs capteurs; calibrage et synchronisation temporelle précis.

Marges de sécurité dépendantes de la vitesse et de l’incertitude.

Latence bout-en-bout maîtrisée; replanification rapide sous surcharge.

Tests en simulation, relecture de logs, scénarios edge cases (obstacles fins, lumière basse, pluie, surfaces brillantes).

*/
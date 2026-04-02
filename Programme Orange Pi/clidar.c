#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
//#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h> 

// gcc -Wall clidar.c -o clidar
//ctrl / pour dé/commenter

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
#define RESET_BYTE 0x40
#define INFO_LEN 20
#define HEALTH_LEN 3
#define RESET 0x40
#define SCAN_BYTE 0x20

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
		perror(" New flag mismatch ");
	}
	uint8_t check_bit = raw[1] & 0b1;
	if (check_bit != 1){
		perror(" check_bit not equal to 1 ");
	}
	float angle = ((raw[1] >> 1) + (raw[2] << 7)) / 64. ;
	float distance = (raw[3] + (raw[4] <<8)) / 4. ;
	
	float data[3] = {angle, distance, quality} ;
	
	return *data;
}

int LidarConnect(char pathlidar[13], struct lidar *self){ 

	(*self).fd = 3;
	printf("LidarConnect pathlidar : %s \n", pathlidar);
	printf("LidarConnect fd : %d \n", (*self).fd);
	
	return 0;
}

// int LidarConnect (char pathlidar[], struct lidar *self) {
// 	/*Function to set the uart communication
// 	//Also try with those parameters for the ildar setting 

// 	(*self).fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
// 	if((*self).fd < 0){
// 		perror("Error Open UART bus");
// 		return -1;
// 	}
// 	printf("Uart open\n");
	
//     struct termios options;

//     if(tcgetattr((*self).fd, &options) < 0){
//   		printf("Err tcgetattr\n");
//   		close((*self).fd);
//  	}

//     cfsetospeed(&options, B115200);
//     cfsetispeed(&options, B115200);
//     options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;
//     options.c_lflag = 0;
//     options.c_oflag = 0;
//     options.c_cc[VMIN] = 0; 
// 	options.c_cc[VTIME] = 10; // 1s timeout
//     if(tcsetattr((*self).fd, TCSANOW, &options) < 0 ){
// 		perror("Setting serial parameters'Failed to connect to the sensor \n");
// 		close((*self).fd);
// 		return -1;
// 	}
// 	printf("serial set\n");

// 	*/
    
// 	struct termios options;

//     (*self).fd = open(pathlidar, O_RDWR );
// 	if((*self).fd < 0){
// 		perror("Error Open UART bus");
// 		return -1;
// 	}
// 	printf("Uart open\n");

//     //PARITY_NONE, STOPBITS_ONE

//     if(tcgetattr((*self).fd, &options) < 0){
//   		printf("Err tcgetattr\n");
//   		close((*self).fd);
//  	}

// 	options.c_cflag = B460800 | CS8 | CREAD | CLOCAL; //baudrade, data size
// 	options.c_iflag = 0;
// 	options.c_oflag = 0;
// 	options.c_lflag = 0;

// 	//apply the settings
// 	tcflush((*self).fd, TCIFLUSH);
// 	if(tcsetattr((*self).fd, TCSANOW, &options) < 0 ){
// 		perror("Setting serial parameters'Failed to connect to the sensor \n");
// 		close((*self).fd);
// 		return -1;
// 	}
// 	printf("serial set\n");

// 	return 0;

// }

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
	
	while(len < 4){ //need to had a watchdog on this measurement
		len = read(fd, data, lendata );
	}
	for(int i =0; i<= lendata; i++){
        value[i] = data[i];
    }
	
}

// int _send_payload_cmd(int fd, int cmd, uint8_t *payload){
// 	/*
// 	Function that add the payload value to the data before send to lidar 
// 	*/
// 	uint8_t size = 0x02 ;
// 	uint8_t req[6] = {SYNC_BYTE, cmd, size, *payload} ;
// 	uint8_t checksum;
// 	checksum = 193;

// 	req[5]= checksum ;
	
// 	_send_cmd(fd, req);

// 	return 0;
// } 

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
	
	uint8_t valeurs[5], descriptor[7];

	_send_cmd(self.fd, SCAN_BYTE);
	if (read(self.fd, descriptor, 7) != 7 || descriptor[0] != 0xA5 || descriptor[1] != 0x5A) {
        printf("Failed to receive valid response descriptor[cite: 183].\n");
        return -1;
    }

	if(read(self.fd, valeurs, 5) == 5){

	float angle = (((uint16_t)valeurs[2] << 7) | (valeurs[1] >> 1)) / 64.0f; // [cite: 413]
	float dist = (((uint16_t)valeurs[4] << 8) | valeurs[3]) / 4.0f;          // [cite: 414]
	printf("Angle: %.2f deg, Dist: %.2f mm\n", angle, dist);
	}else{
		printf("Invalid packet detected, skipping...\n");
		
	}

	// _read_descriptor(valeurs);
	// *valeurs =_process_scan(valeurs);

	// *angle = valeurs[0]; 
	// *distance = valeurs[1];
	// *quality = valeurs[2];

	return 0;
}

void reset(uint8_t fd){

	_send_cmd(fd, RESET);

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

	uint8_t desc[7], info[20];
    if (read(fd, desc, 7) == 7 && desc[0] == 0xA5) {
        read(fd, info, 20);
        // C1 MajorModel is 4 
        printf("Model ID: %d, Firmware: %d.%d\n", info[0], info[2], info[1]);
        printf("Serial: ");
        for(int i=0; i<16; i++) printf("%02X", info[4+i]);
        printf("\n");
    }

	*/
	printf("Get Infos \n");

	uint8_t raw[20];
	char temp[255];
	
	_send_cmd((*self).fd, GET_INFO_BYTE);
	_read_raw((*self).fd, raw, INFO_LEN);
	
	for(int i =0; i < sizeof(raw); i++){
		printf(" raw %d : %x %d \n", i, raw[i], raw[i] );
	}

	_read_descriptor(raw);

	sprintf(temp, "%d",raw[0]);
	strcpy((*self).info.model, temp);
	sprintf(temp, "%d",raw[2]+raw[1]);
	strcpy((*self).info.firmware, temp);
	sprintf(temp, "%d",raw[3]);
	strcpy((*self).info.hardware, temp);
	sprintf(temp, "%d",raw[4]);
	strcpy((*self).info.serial_number, temp);
	
}

void  stop(int fd){
	printf("Stop function\n");
	_send_cmd(fd, STOP_BYTE);
	usleep(1*100);
	
}

int main (void){

	struct lidar new_lidar;
	int error_code = 0;

	if( LidarConnect("/dev/ttyAMA0", &new_lidar) <0){
		perror("Lidar connexion failed ");
	}

	getinfo(&new_lidar);

	printf("model : %s \n", new_lidar.info.model);
	printf("firmware : %s\n", new_lidar.info.firmware);
	printf("hardware : %s\n", new_lidar.info.hardware);
	printf("serial_number : %s\n", new_lidar.info.serial_number);

	reset(new_lidar.fd);
	
	if(gethealth(new_lidar)!=0){
		printf("Error code : %d \n", error_code);
		LidarDeconnect(new_lidar.fd);
	}
	
	for(int i=0; i<10; i++){
		iter_measurement(new_lidar); 
	}

	stop(new_lidar.fd);
	stop_motor(new_lidar.fd);
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
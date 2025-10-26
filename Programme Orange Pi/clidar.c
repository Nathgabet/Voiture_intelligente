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

#define SYNC_BYTE 0xA5
#define SYNC_BYTE2 0x5A
#define GET_INFO_BYTE  0x50
#define GET_HEALTH_BYTE  0x52
#define DESCRIPTOR_LEN 7
#define MAX_MOTOR_PWM  1023
#define DEFAULT_MOTOR_PWM  660
#define SET_PWM_BYTE 0xF0 //register related to the 
#define STOP_BYTE 0x25
#define RESET_BYTE 0x40
#define INFO_LEN 20
#define HEALTH_LEN 3

struct lidardata{

	float angle;
	float distance;
	float quality;
};

struct lidarinfo {

	char model[255];
	char firmware[255];
	char hardware[255];
	char serial_number[255];
		
};

struct lidar{

	int fd;
    int motor_speed ;
    int express_trame ;
    uint8_t express_data ;
    uint8_t motor_running ;
    uint8_t connect;
	uint8_t scanning;
	struct lidarinfo info;
	struct lidardata data;
    
};

float  _process_scan(int *raw[]){
	/*Function that extract the diffrents value of the data
	Args:
		raw (int): raw data
	Returns:
		data (float): char of the angle, distance and quality of mesurement
	*/
	uint8_t new_scan = *raw[0] &  0b1;
	uint8_t invert_new_scan = (*raw[0] >> 1) &  0b1;
	uint8_t quality = *raw[0] >> 2 ;
	
	if (new_scan == invert_new_scan){
		printf(" New flag mismatch \n");
	}
	uint8_t check_bit = *raw[1] & 0b1;
	if (check_bit != 1){
		printf(" check_bit not equal to 1 \n");
	}
	float angle = ((*raw[1] >> 1) + (*raw[2] << 7)) / 64. ;
	float distance = (*raw[3] + (*raw[4] <<8)) / 4. ;
	
	float data[3] = {angle, distance, quality} ;
	
	return *data;
}

// void LidarConnect(char pathlidar[13], struct lidar *self){ 

// 	(*self).fd = 3;
// 	printf("\n LidarConnect pathlidar : %s", pathlidar);
// 	printf("\n LidarConnect fd : %d", (*self).fd);
	
// }

int LidarConnect (char pathlidar[], struct lidar *self) {
	/*Function to set the uart communication
	
	*/
    
	struct termios options;

    (*self).fd = open(pathlidar, O_RDWR );
	if((*self).fd < 0){
		perror("Error Open UART bus");
		return -1;
	}
	printf("Uart open\n");

    //PARITY_NONE, STOPBITS_ONE

    if(tcgetattr((*self).fd, &options) < 0){
  		printf("Err tcgetattr\n");
  		close((*self).fd);
 	}

	options.c_cflag = B460800 | CS8 | CREAD | CLOCAL; //baudrade, data size
	options.c_iflag = 0;
	options.c_oflag = 0;
	options.c_lflag = 0;

	//apply the settings
	tcflush((*self).fd, TCIFLUSH);
	if(tcsetattr((*self).fd, TCSANOW, &options) < 0 ){
		perror("Setting serial parameters'Failed to connect to the sensor \n");
		close((*self).fd);
		return -1;
	}
	printf("serial set\n");

	return 0;

}

void _send_raw(int fd, uint8_t *data, int len ){
	/*Function to send data to the lidar
	Args:
		fd(int): forward directory 
		data (int): data to send to the lidar
		len (int): len of the data
	*/
	
	len = write(fd, data, len);
	printf("Send Request lidar info %x len:%ld", data[1], sizeof(len));
}

void _read_raw(struct lidar self, int *value, int lendata){
	/*Function to received the raw data of the lidar
		Args:
			self (struct): structur type lidar 
			lendata (int) len of the data to read
		Returns :
			value (int): return the value readed
	*/

	uint8_t len = 0;
	
	do{ //need to had a watchdog on this measurement
		
		len = read(self.fd, *value, lendata );
	
	}while(len < 4);

	printf("\n_read raw Data\n");
}

int _send_payload_cmd(int fd, int cmd, uint8_t *payload){
	/*Function that add the payload value to the data before send to lidar 
	Args:
		fd (int) : forward directory 
		cmd (int) :
		payload (uint8_t): 
	*/
	uint8_t size = 0x02 ;
	uint8_t req[6] = {SYNC_BYTE, cmd, size, *payload} ;
	uint8_t checksum;
	checksum = 193;

	req[5]= checksum ;
	
	_send_raw(fd, req, 6);

	return 0;
} 

int _send_cmd(int fd, int cmd){

	uint8_t req[2] = {SYNC_BYTE, cmd};

	printf("\nlen of the sent request : %ld and data : %x %x", sizeof(req), req[0], req[1] );
	_send_raw(fd, req, 2);

    return 0;
}

int LidarDeconnect (int fd){
	printf("\nLidar deconnect");
    close(fd);

    return 0;
}

int motor_speed(struct lidar self){

	//self._set_pwm(self._motor_speed)
    //self.motor_running = True

	if ( (0< self.motor_speed) || (self.motor_speed > MAX_MOTOR_PWM)){
		perror("\nMotor speed not right\n");
		return -1;
	}
	if (self.motor_running){
		uint8_t payload[2] = {0x03, self.motor_speed} ; //data et apres, 0x03 
		_send_payload_cmd(self.fd, SET_PWM_BYTE, payload);//Register related to the motor

	}

	return 0;
}

int start_motor(struct lidar self){

	printf("\nStar motor");
	motor_speed(self);

	return 0;
}

int stop_motor(struct lidar self){
	printf("\nStop motor");
	self.motor_speed = 0;
	motor_speed(self);

	return 0;
}

int iter_measurement (struct lidar self, struct lidardata *data ){
	int *trame;
	float *valeurs;
	
	start(self);

	*trame = _read_descriptor(&self);
	//*valeurs =_process_scan(trame);//il lui faut un tableau de 3 cases

	(*data).angle = valeurs[0]; 
	(*data).distance = valeurs[1];
	(*data).quality = valeurs[2];
	return 0;
}

int _read_descriptor(struct lidar *self){
	int *descriptor = 0;
	
	_read_raw(*self, descriptor, DESCRIPTOR_LEN);//need to check the implémentation of the différente value

	printf("\nsizeof(descriptor) : %ld", sizeof(descriptor));
	if ( sizeof(descriptor) != DESCRIPTOR_LEN){
		perror("\nDescriptor length mismatch");
		return -1;
	}
	else if (((descriptor[0] == SYNC_BYTE) &&  ( descriptor[1] ==  SYNC_BYTE2))){
		perror("\nNo sync byte");
		return -1;
	}
	for(int i= 0; i<4; i++){
		descriptor[i] = descriptor[i+1];
	}
	return descriptor;
}

uint8_t gethealth(struct lidar self){
	/*Get device health state. When the core system detects some
        potential risk that may cause hardware failure in the future,
        the returned status value will be 'Warning'. But sensor can still work
        as normal. When sensor is in the Protection Stop state, the returned
        status value will be 'Error'. In case of warning or error statuses
        non-zero error code will be returned.

        Returns
        -------
        status : uint8_t
            0 = 'Good', 1 = 'Warning' or 2 = 'Error' statuses
        error_code : uint8_t
            The related error code that caused a warning/error.
    */
    int *dsize;
    int raw[7];
    int error_code;
	
	perror("\n Gethealth");

	_send_cmd(self.fd, GET_HEALTH_BYTE);
	*dsize = _read_descriptor(&self);
	if (sizeof(*dsize) != HEALTH_LEN){
		perror("\nHealth_len not mathching");
		return -1;
	}
	
	_read_raw(self, raw, HEALTH_LEN);
	printf("\nhealth statue : %d", raw[0]);
	error_code = (raw[1] <<8) + raw[2];

	return error_code;
}

void getinfo(struct lidar *self){
	printf("\nGet Infos ");
	int dsize, raw[5];
	char temp[255];
	
	_send_cmd((*self).fd, GET_INFO_BYTE);
	_read_raw(*self, raw, 8);
	
	for(int i =0; i < sizeof(raw); i++){

		printf(" raw %d : %x %d \n", i, raw[i], raw[i] );

	}

	sprintf(temp, "%d",raw[0]);
	strcpy((*self).info.model, temp);
	sprintf(temp, "%d",raw[2]+raw[1]);
	strcpy((*self).info.firmware, temp);
	sprintf(temp, "%d",raw[3]);
	strcpy((*self).info.hardware, temp);
	sprintf(temp, "%d",raw[4]);
	strcpy((*self).info.serial_number, temp);
	
}

int start(struct lidar self){
	/*Start the scanning process

        Parameters
        ----------
        scan : normal, force or express.*/

	uint8_t error_code;

	if( self.scanning == 1){

		printf("\nScanning already running");
		return 0;
	}
	error_code = gethealth(self);

	return error_code;
}

void  stop(struct lidar self){
	printf("\nStop function");
	_send_cmd(self.fd, STOP_BYTE);
	usleep(1*100);
	
}

int main (void){

	/*
	from rplidar import RPLidar
	lidar = RPLidar('/dev/ttyUSB0')

	info = lidar.get_info()
	print(info)
	health = lidar.get_health()
	print(health)

	for i, scan in enumerate(lidar.iter_scans()):
		print('%d: Got %d measures' % (i, len(scan)))
		if i > 10:
			break

	lidar.stop()
	lidar.stop_motor()
	lidar.disconnect()
	*/

	struct lidar new_lidar;
	int error_code = 0;

	LidarConnect("/dev/ttyAMA0", &new_lidar);
	
	getinfo(&new_lidar);//need to check the read raw 

	printf("\n model : %s", new_lidar.info.model);
	printf("\n firmware : %s", new_lidar.info.firmware);
	printf("\n hardware : %s", new_lidar.info.hardware);
	printf("\n serial_number : %s", new_lidar.info.serial_number);

	//error_code = gethealth(new_lidar);//genere une Segmentation fault
	if(error_code!=0){
		printf("\nError code : %d", error_code);
		LidarDeconnect(new_lidar.fd);
	}
	
	start_motor(new_lidar); //fait et fini
	for(int i=0; i<10; i++){
		//iter_measurement(new_lidar, &(new_lidar.data)); //genere une Segmentation fault
		printf("\n %d : angle: %f, distance : %f, quality : %f", i, new_lidar.data.angle,new_lidar.data.distance, new_lidar.data.quality);
	}

	//Envoyer les données dans un fichier qui ser ouvert par un programme en python qui traitera les données


	stop(new_lidar);
	stop_motor(new_lidar);
	LidarDeconnect(new_lidar.fd);

    return 0;
}

/*
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
#ifndef LIDAR_H
#define LIDAR_H

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

#include <unistd.h>

typedef struct {
	uint8_t fd;
    uint8_t motor_speed ;
	char errorname;
	uint8_t errorcode;
	double error_count;
	float angle;
	float distance;
	uint8_t quality;
	pthread_t handle_thread;
}lidar;

uint16_t gethealth();
uint8_t checksum(uint8_t *packet, int packet_len);
float process_scan(uint8_t *raw);
int LidarConnect (char pathlidar[]);
int _read_descriptor(uint8_t *raw_data);
void _send_cmd(uint8_t cmd);
uint8_t _read_raw(uint8_t *buffer, int length); 
int LidarDeconnect ();
int motor_speed(uint16_t rpm);
int start_motor();
int stop_motor();
void get_samplerate();
void *loop_measurement(void *arg);
void iter_measurement();
void reset();
uint16_t gethealth();
void getinfo();
void stop_everything();
int startreadmesurement();
int LidarInit();
void report(double number_iteration, long int timepast);








#endif
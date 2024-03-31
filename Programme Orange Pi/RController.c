#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <linux/joystick.h>
#include <linux/gpio.h>
#include "lxlib.h"

void *arret( void *arg){

	const char *device;
    	int js;
    	struct js_event *event = (struct js_event *) arg;
    	struct axis_state axes[3] = {0};
    	size_t axis;

        device = "/dev/input/js0";
	js = open(device, O_RDONLY);
    	if ( js <0){
        	perror("Could not open joystick");
	}

    	while (read_event(js, event) == 0){

		switch ((*event).type){
	        	case JS_EVENT_BUTTON:
					printf("Button %u %s\n", (* event).number, (* event).value ? "pressed" : "released");
                	break;
            		case JS_EVENT_AXIS:
                		axis = get_axis_state(event, axes);
                		if (axis < 3)
                    			printf("Axis %zu at (%6d, %6d)\n", axis, axes[axis].x, axes[axis].y);
                		break;
            		default:
                		break;
        	}

        	fflush(stdout);
    	}

	close(js);
}

int main (int argc, char *argv[]) {

	struct js_event wireless;
	pthread_t t1;
	int rond =1;

	if(pthread_create(&t1, NULL, *arret, &wireless)){
                perror("Error: Creation du thread");
                return -1;
        }

	while(rond){

		if(wireless.type == JS_EVENT_BUTTON && wireless.number ==0 && wireless.value ==1){
			printf(" croix  pressed\n");
		}

		if(wireless.type == JS_EVENT_BUTTON && wireless.number ==1  && wireless.value ==1){
                        printf(" rond  pressed\n");
			rond =0;
                }

		 if(wireless.type == JS_EVENT_BUTTON && wireless.number ==2  && wireless.value ==1){
                        printf(" triangle  pressed\n");
                }

		 if(wireless.type == JS_EVENT_BUTTON && wireless.number ==3  && wireless.value ==1){
                        printf(" carre  pressed\n");
                }

	}

	return 0;
}

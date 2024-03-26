#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <linux/joystick.h>

int read_event(int fd, struct js_event *event){

        ssize_t bytes;

        bytes = read(fd, event, sizeof(*event));

        if (bytes == sizeof(*event))
                return 0;


        return -1;
}

size_t get_axis_count(int fd){

        uint8_t axes;

        if (ioctl(fd, JSIOCGAXES, &axes) == -1)
                return 0;

        return axes;
}

size_t get_button_count(int fd){

        uint8_t  buttons;

        if (ioctl(fd, JSIOCGBUTTONS, &buttons) < 0)
                return 0;

        return buttons;
}

struct axis_state {

        short x, y;

};

size_t get_axis_state(struct js_event *event, struct axis_state axes[3]){

        size_t axis = event->number / 2;

        if (axis < 3){

                if (event->number % 2 == 0)
                        axes[axis].x = event->value;
                else
                        axes[axis].y = event->value;
        }

    return axis;
}

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

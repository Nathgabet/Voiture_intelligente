#include <stdio.h>
#include <wiringPi.h>

// LED Pin - wiringPi pin 0 is BCM_GPIO 17.

#define LED 26

int boucle = 1;

PI_THREAD (flag){

        int value =0;

        for (;;){
                value = digitalRead (LED) ;  // On
                delay (10) ;               // mS
                if (value == 1){
                        boucle = 0;
                        return 0;
                }
                else{
                        boucle = 1;
                }
        }
}


int main (void){

        printf ("Raspberry Pi blink\n") ;

        wiringPiSetup () ;
        pinMode (LED, INPUT) ;

        piThreadCreate (flag) ;

        do{
                printf ("La led est eteint\n") ;
                delay (700);

        }while(boucle);
        printf ("La led s'est allumée\n") ;

        return 0 ;
}

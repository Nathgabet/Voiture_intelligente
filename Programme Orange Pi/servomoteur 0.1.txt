
/*

Programme permettant l'utilisation du servomoteur selon les réglages
de la voiture RC

*****************************************************************************

Les valeurs limites sont :
+ - - - + - - + - - + - - + - - -  +
|       | MAX | MID | MIN |        |
+ - - - + - - + - - + - - + - - -  +
|GAUCHE | 186 | 146 | 126 | DROITE |
+ - - - + - - + - - + - - + - - -- +
 
*****************************************************************************
Duty cycle:
	servomoteur
9.31% - 7.33% - 5.31

	moteur puissance max (-20% pmin)
5.00% - 7.50% - 10.00%

*****************************************************************************

Compiler le code :

gcc -Wall servomoteur.c -lwiringPi -o servomoteur


*/

#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#define PWM 21

int main(void){

        printf ("OrangePi Pi wiringPi PWM test program\n") ;

        if (wiringPiSetup () == -1)
                exit (1) ;

        pinMode (PWM, PWM_OUTPUT) ;

        pwmSetRange(PWM,2000);
        pwmSetClock(PWM,240);

        pwmWrite (PWM, 500) ;
        delay(2000);

        pinMode (PWM, OUTPUT) ;
        digitalWrite(PWM, LOW);

        return 0;
}


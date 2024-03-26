/*

Programme permettant de choisir la valeur du PWM

*****************************************************************************

Les valeurs limites sont :
+ - - - - + - - + - - + - - +
|         | MAX | MID | MIN |
+ - - - - + - - + - - + - - +
|  AVANT  | 200 | 160 | 150 |
+ - - - - + - - + - - + - - +
| ARRIERE | 100 | 136 | 150 |
+ - - - - + - - + - - + - - +
*****************************************************************************
Duty cycle:
	servomoteur
9.31% - 7.33% - 5.31

	moteur puissance max (-20% pmin)
5.00% - 7.50% - 10.00%

*****************************************************************************

*/

#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#define PWM 21

int main(void){

        if (wiringPiSetup () == -1)
                exit (1) ;

        int duty, sortie1 = 1;

        pinMode (PWM, PWM_OUTPUT) ;

        pwmSetRange(PWM,2000);
        pwmSetClock(PWM,240);

        do{
                do{
                        printf("Entrer le duty cycle voulue entre [35,265] \n");
                        scanf("%d", &duty);

                        if(duty< 35 || duty>265)
                                printf("Valeur incorrecte, recommencer\n");
                        else
                                sortie1 = 0;

                }while(sortie1);

                pwmWrite (PWM, duty) ;
                sortie1 = 1;
        }while(1);

        delay(2000);

        pinMode (PWM, OUTPUT) ;
        digitalWrite(PWM, LOW);

        return 0;

}

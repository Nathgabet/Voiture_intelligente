#include <stdio.h>
#include <wiringPi.h>

// LED Pin - wiringPi pin 0 is BCM_GPIO 17.

#define LED 22

PI_THREAD (blinky)
{
  for (;;)
  {
    digitalWrite (LED, HIGH) ;  // On
    delay (200) ;               // mS
    digitalWrite (LED, LOW) ;   // Off
    delay (400) ;
  }
}


int main (void)
{
  printf ("Raspberry Pi blink\n") ;

  wiringPiSetup () ;
  pinMode (LED, OUTPUT) ;

  piThreadCreate (blinky) ;

  for (;;)
  {
    printf ("Hello, world\n") ;
    delay (700) ;
  }

  return 0 ;
}

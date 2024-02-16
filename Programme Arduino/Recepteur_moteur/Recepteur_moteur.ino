/*
Receveur (composent le plus long) fil vert sur le pin 11  
Emetteur (composent le plus court) fil bleu sur le pin 12
*/

/*
Min pulse width 0°  : 750
Mid pulse width 90° : 1500
Max pulse width 180°: 2250
*/

#include <RH_ASK.h>
#include <SPI.h>
#include "ServoTimer2.h"

RH_ASK rf_driver;
ServoTimer2 Servo1;
ServoTimer2 Servo2;

void setup() {
  
  Servo2.attach(5);
  Servo1.attach(7);
  
  rf_driver.init();
  Serial.begin(9600);

}

void loop() {

  int pot1, pot2, pot3, pot4;
  String str_pot1, str_pot2, str_pot3, str_pot4, str_out;
  float entre_min = 100 , entre_max = 800, sortie_min = 750 , sortie_max = 2250;
  int servomoteur , moteur ;
  uint8_t buf[16];
  uint8_t buflen = sizeof(buf);

  if (rf_driver.recv(buf, &buflen)){
    str_out = String((char*) buf);

    for(int i = 0; i<str_out.length(); i++){
     
      str_pot1 = str_out.substring(0,3);
      str_pot2 = str_out.substring(3,6);
      str_pot3 = str_out.substring(6,9);
      str_pot4 = str_out.substring(9,12);
        
    }
  
  Mo( &pot1, &pot2, &pot3, &pot4, str_pot1, str_pot2, str_pot3, str_pot4 );

    pot1 = str_pot1.toInt();
    pot2 = str_pot2.toInt();
    pot3 = str_pot3.toInt();
    pot4 = str_pot4.toInt();

    Serial.print(pot1);
    Serial.print("  ");
    Serial.print(pot2);
    Serial.print("  ");
    Serial.print(pot3);
    Serial.print("  ");
    Serial.print(pot4);
    Serial.print("  ");
    Serial.println(buflen);   
  
    servomoteur = (pot1-entre_min) * (sortie_max - sortie_min) / (entre_max-entre_min) + sortie_min;
    moteur = (pot2-entre_min) * (sortie_max - sortie_min) / (entre_max-entre_min) + sortie_min;

    Serial.print(servomoteur);
    Serial.print("  ");
    Serial.println(moteur); 

    Servo1.write(servomoteur);
    Servo2.write(moteur);
    
    


  }


}

void Mo(int *pot1, int *pot2, int *pot3, int *pot4, String str_pot1, String str_pot2, String str_pot3, String str_pot4 ){
 
  *pot1 = str_pot1.toInt();
  *pot2 = str_pot2.toInt();
  *pot3 = str_pot3.toInt();
  *pot4 = str_pot4.toInt();

}

/*
Receveur (composent le plus long) fil vert sur le pin 11  
Emetteur (composent le plus court) fil bleu sur le pin 12
*/
#include <RH_ASK.h>
#include <SPI.h>
#include "ServoTimer2.h"

RH_ASK rf_driver;

ServoTimer2 servo;
ServoTimer2 moteur;

int pot1;
int pot2;

String str_pot1;
String str_pot2;

String str_out;

void setup() {
  
  servo.attach(5);

  rf_driver.init();
  Serial.begin(9600);

}

void loop() {
  
  uint8_t buf[7];
  uint8_t buflen = sizeof(buf);

  if (rf_driver.recv(buf, &buflen)){
    str_out = String((char*) buf);

    for(int i = 0; i<str_out.length(); i++){
      if (str_out.substring(i, i+1) == ",") {
        str_pot1 = str_out.substring(0,i);
        str_pot2 = str_out.substring(i+1);
        break;
      }
     
    }
    
    Serial.print(str_pot1);
    Serial.print("  ");
    Serial.println(str_pot2);
        
  }

}


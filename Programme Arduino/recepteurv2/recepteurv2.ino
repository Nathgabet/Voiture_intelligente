/*
Receveur (composent le plus long) fil vert sur le pin 11  
Emetteur (composent le plus court) fil bleu sur le pin 12
*/
#include <RH_ASK.h>
#include <SPI.h>
#include "ServoTimer2.h"

RH_ASK rf_driver;
ServoTimer2 servo;

void setup() {
  
  //servo.attach(15);
  rf_driver.init();
  Serial.begin(9600);

}

void loop() {

  int pot1, pot2, pot3, pot4;
  String str_pot1, str_pot2, str_pot3, str_pot4, str_out;
  uint8_t buf[16];
  uint8_t buflen = sizeof(buf);

  if (rf_driver.recv(buf, &buflen)){
    str_out = String((char*) buf);

    Serial.print(str_out);
    Serial.print("  ");
    Serial.println(buflen);

    for(int i = 0; i<str_out.length(); i++){
     
      str_pot1 = str_out.substring(0,4);
      str_pot2 = str_out.substring(4,8);
      str_pot3 = str_out.substring(8,12);
      str_pot4 = str_out.substring(12,16);
        
      }

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
      
  }

}
/*
void Mo(int pot1, int pot2, int pot3, int pot4, String *str_pot1, String *str_pot2, String *str_pot3, String *str_pot4 ){
  

  if (*str_pot13 <10){
    *str_pot1 = "999" + *str_pot1;
  }
  else if (pot1 <100){
    *str_pot1 = "99" + *str_pot1;
  }
  else if (pot1 <1000){
    *str_pot1 = "9" + *str_pot1;
  }

  if (pot2 <10){
    *str_pot2 = "999" + *str_pot2;
  }
  else if (pot2 <100){
    *str_pot2 = "99" + *str_pot2;
  }
  else if (pot2 <1000){
    *str_pot2 = "9" + *str_pot2;
  }

  if (pot3 <10){
    *str_pot3 = "999" + *str_pot3;
  }
  else if (pot3 <100){
    *str_pot3 = "99" + *str_pot3;
  }
  else if (pot3 <1000){
    *str_pot3 = "9" + *str_pot3;
  }

  if (pot4 <10){
    *str_pot4 = "999" + *str_pot4;
  }
  else if (pot4 <100){
    *str_pot4 = "99" + *str_pot4;
  }
  else if (pot4 <1000){
    *str_pot4 = "9" + *str_pot4;
  }
  Serial.print("  ");
  Serial.print(*str_pot1);
  Serial.print("  ");
  Serial.print(*str_pot2);
  Serial.print("  ");
  Serial.print(*str_pot3);
  Serial.print("  ");
  Serial.print(*str_pot4);
  Serial.println("  ");

}*/

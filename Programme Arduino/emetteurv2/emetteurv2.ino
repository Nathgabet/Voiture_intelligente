/*
Receveur (composent le plus long) fil vert sur le pin 11  
Emetteur (composent le plus court) fil bleu sur le pin 12
*/
#include <RH_ASK.h>
#include <SPI.h>
#define D5 5

RH_ASK rf_driver;

void setup() {
  pinMode(D5, OUTPUT);
  rf_driver.init();
  Serial.begin(9600);
 
}

void loop() {

  int pot1, pot2, pot3, pot4;
  String str_pot1, str_pot2, str_pot3, str_pot4, str_out;

  pot1 = analogRead(A0);
  pot2 = analogRead(A1);
  pot3 = analogRead(A3);
  pot4 = analogRead(A4);

  Mo(pot1, pot2, pot3, pot4, &str_pot1, &str_pot2, &str_pot3, &str_pot4); 
  str_out = str_pot1 + str_pot2 + str_pot3 + str_pot4 ;

 
  Serial.print(str_out);
  Serial.println("  ");

  TxMessage(str_out);
  
  delay(100);
}

void Mo(int pot1, int pot2, int pot3, int pot4, String *str_pot1, String *str_pot2, String *str_pot3, String *str_pot4 ){
  
  *str_pot1 = String(pot1);
  *str_pot2 = String(pot2);
  *str_pot3 = String(pot3);
  *str_pot4 = String(pot4);

  if (pot1 <10){
    *str_pot1 = "***" + *str_pot1;
  }
  else if (pot1 <100){
    *str_pot1 = "**" + *str_pot1;
  }
  else if (pot1 <1000){
    *str_pot1 = "*" + *str_pot1;
  }

  if (pot2 <10){
    *str_pot2 = "***" + *str_pot2;
  }
  else if (pot2 <100){
    *str_pot2 = "**" + *str_pot2;
  }
  else if (pot2 <1000){
    *str_pot2 = "*" + *str_pot2;
  }

  if (pot3 <10){
    *str_pot3 = "***" + *str_pot3;
  }
  else if (pot3 <100){
    *str_pot3 = "**" + *str_pot3;
  }
  else if (pot3 <1000){
    *str_pot3 = "*" + *str_pot3;
  }

  if (pot4 <10){
    *str_pot4 = "***" + *str_pot4;
  }
  else if (pot4 <100){
    *str_pot4 = "**" + *str_pot4;
  }
  else if (pot4 <1000){
    *str_pot4 = "*" + *str_pot4;
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

}

void TxMessage(String str_out){

  digitalWrite(D5,255);

  static char *msg = str_out.c_str();
  rf_driver.send((uint8_t *)msg, strlen(msg));//il faut connetre la taille du message que l'on envoie
  rf_driver.waitPacketSent();
  Serial.print(msg);
  Serial.print("  ");
  Serial.println(strlen(msg));

  digitalWrite(D5,LOW);
}

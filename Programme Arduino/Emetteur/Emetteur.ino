/*
Receveur (composent le plus long) fil vert sur le pin 11  
Emetteur (composent le plus court) fil bleu sur le pin 12
*/
#include <RH_ASK.h>
#include <SPI.h>
RH_ASK rf_driver;

int moteur;
int servo;

String str_moteur;
String str_servo;
String str_out;

void setup() {
  // put your setup code here, to run once:
  rf_driver.init();
  Serial.begin(9600);
 
}

void loop() {

  moteur = analogRead(A0);
  servo = analogRead(A1);
  
  str_moteur = String(moteur);
  str_servo = String(servo);
  str_out = str_moteur + "," + str_servo ;
  //str_out = "1000";
  static char *msg = str_out.c_str();
  rf_driver.send((uint8_t *)msg, strlen(msg));//il faut connetre la taille du message que l'on envoie
  rf_driver.waitPacketSent();
  
  Serial.print(str_servo);
  Serial.print("  ");
  Serial.print(str_moteur);
  Serial.print("  ");
  Serial.print(msg);
  Serial.print("  ");
  Serial.println(strlen(msg));

  delay(100);
}

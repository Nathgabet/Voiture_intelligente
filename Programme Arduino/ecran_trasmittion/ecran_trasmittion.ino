/*
Receveur (composent le plus long) fil vert sur le pin 11  
Emetteur (composent le plus court) fil bleu sur le pin 12
*/
#include <RH_ASK.h>
#include <SPI.h>
#include "DFRobot_GDL.h"
#include <SPI.h>

#define CS_Screen 3
#define DC_Screen 2
#define TR_Screen 4
#define D5 5

RH_ASK rf_driver;
DFRobot_ST7789_240x240_HW_SPI screen(DC_Screen,CS_Screen,TR_Screen);

typedef int potentiometre[2];

uint8_t x=3;
uint16_t y=24;
uint8_t heigth =10;
uint16_t width = 201;

void setup() {
  pinMode(D5, OUTPUT);
  rf_driver.init();
  Serial.begin(9600);
    
  screen.begin();
  screen.fillScreen(COLOR_RGB565_WHITE); 
  
}

void loop() {

  int pot1, pot2, pot3, pot4;
  String str_pot1, str_pot2, str_pot3, str_pot4, str_out;

  //potar( &pot1, &pot2, &pot3, &pot4);
  
  menu();

  
  //Affiche(pot1, pot2, pot3, pot4);
  //Mo(pot1, pot2, pot3, pot4, &str_pot1, &str_pot2, &str_pot3, &str_pot4); 
  //str_out = str_pot1 + str_pot2 + str_pot3 + str_pot4;
 
 // TxMessage(str_out);
  
  delay(100);
}

void InitPot(){

  screen.fillScreen(COLOR_RGB565_WHITE);  //font d'ecran une seule couleur
  screen.setTextColor(COLOR_RGB565_BLACK);
  screen.setTextSize(2);

  for (int i = 0; i<4; i++){
    screen.setCursor(/*x=*/3,/*y=*/4 + 33*i);
    screen.print("Potentiometre");
    screen.setCursor(175,4 +33*i);
    screen.print(i+1);
    screen.drawRect(x, y+33*i , width, heigth, COLOR_RGB565_BLACK);
  }
}

void TxMessage(String str_out){

  digitalWrite(D5,HIGH);

  static char *msg = str_out.c_str();
  rf_driver.send((uint8_t *)msg, strlen(msg));//il faut connetre la taille du message que l'on envoie
  rf_driver.waitPacketSent();
  digitalWrite(D5,LOW);
 /*
  Serial.print(msg);
  Serial.print("  ");
  Serial.println(strlen(msg));
  */
 
}

void menu(){
  
  int statement = 1, pot1 = 500,pot3 = 600;
  screen.fillScreen(COLOR_RGB565_WHITE);  //font d'ecran une seule couleur
  screen.setTextColor(COLOR_RGB565_BLACK);
  screen.setTextSize(2);
  
  screen.setCursor(10,/*y=*/37);
  screen.print("Potentiometres");
  
  screen.setCursor(10,77);
  screen.print("Images");
    
  screen.setCursor(10,117);
  screen.print("Test communication");
  
  screen.setCursor(10,157);
  screen.print("Initialisation");

  while (pot3>350){
    delay(150);
    pot1 = 100 + analogRead(A1)/1.46;
    pot3 = 100 + analogRead(A2)/1.46;
    Serial.print(pot1);
    Serial.print("  ");
    Serial.print(pot3);
    Serial.println("  ");

    if(pot1>600){
      statement +=1;
    }
    else if (pot1<350){
      statement +=1;
    }
    
    if(statement >4){
      statement =1;
    }
    else if (statement <1){
      statement = 4;
    }
    
    switch(statement){

      case 1:
        screen.fillRect(3, 33,4, 20, COLOR_RGB565_BLACK);
        screen.fillRect(3, 73,4, 20, COLOR_RGB565_WHITE);
        screen.fillRect(3, 113,4, 20, COLOR_RGB565_WHITE);
        screen.fillRect(3, 153,4, 20, COLOR_RGB565_WHITE);
        break;
    
      case 2:
        screen.fillRect(3, 33,4, 20, COLOR_RGB565_WHITE);
        screen.fillRect(3, 73,4, 20, COLOR_RGB565_BLACK);
        screen.fillRect(3, 113,4, 20, COLOR_RGB565_WHITE);
        screen.fillRect(3, 153,4, 20, COLOR_RGB565_WHITE);
        break;
      case 3:
        screen.fillRect(3, 33,4, 20, COLOR_RGB565_WHITE);
        screen.fillRect(3, 73,4, 20, COLOR_RGB565_WHITE);
        screen.fillRect(3, 113,4, 20, COLOR_RGB565_BLACK);
        screen.fillRect(3, 153,4, 20, COLOR_RGB565_WHITE);
        break;
      case 4:
        screen.fillRect(3, 33,4, 20, COLOR_RGB565_WHITE);
        screen.fillRect(3, 73,4, 20, COLOR_RGB565_WHITE);
        screen.fillRect(3, 113,4, 20, COLOR_RGB565_WHITE);
        screen.fillRect(3, 153,4, 20, COLOR_RGB565_BLACK);
        break;
      
    }
  }
  screen.fillScreen(COLOR_RGB565_BLUE);
}

void Affiche(int pot1, int pot2, int pot3, int pot4){

  pot1 = pot1/5.07;
  pot2 = pot2/5.07;
  pot3 = pot3/5.07;
  pot4 = pot4/5.07;

 // InitPot();
  
  screen.fillRect(x+1, y+1, pot1, heigth-2, COLOR_RGB565_RED);
  screen.fillRect(x+pot1+1, y+1 , (width-pot1-2), heigth-2, COLOR_RGB565_WHITE);

  screen.fillRect(x+1, y+34 , pot2, heigth-2, COLOR_RGB565_BLUE);
  screen.fillRect(x+pot2+1, y+34 , (width-pot2-2), heigth-2, COLOR_RGB565_WHITE);

  screen.fillRect(x+1, y+67 , pot3, heigth-2, COLOR_RGB565_GREEN );
  screen.fillRect(x+pot3+1, y+67 , (width-pot3-2), heigth-2, COLOR_RGB565_WHITE);

  screen.fillRect(x+1, y+100 , pot4, heigth-2, COLOR_RGB565_ORANGE);
  screen.fillRect(x+pot4+1, y+100 , (width-pot4-2), heigth-2, COLOR_RGB565_WHITE);
/*
  Serial.print(pot1);
  Serial.print("  ");
  Serial.print(pot2);
  Serial.print("  ");
  Serial.print(pot3);
  Serial.print("  ");
  Serial.println(pot4);*/
}

void potar( int *pot1, int *pot2, int *pot3, int *pot4){
  
  *pot1 = analogRead(A0);
  *pot2 =  analogRead(A1);
  *pot3 =  analogRead(A2);
  *pot4 =  analogRead(A3);
  
/*
  Serial.print(*pot1);
  Serial.print("  ");
  Serial.print(*pot2);
  Serial.print("  ");
  Serial.print(*pot3);
  Serial.print("  ");
  Serial.println(*pot4);*/
}

void Mo(int pot1, int pot2, int pot3, int pot4, String *str_pot1, String *str_pot2, String *str_pot3, String *str_pot4 ){
 
  pot1 = 100 + pot1/1.46;
  pot2 = 100 + pot2/1.46;
  pot3 = 100 + pot3/1.46;
  pot4 = 100 + pot4/1.46;
  
  *str_pot1 = String(pot1);
  *str_pot2 = String(pot2);
  *str_pot3 = String(pot3);
  *str_pot4 = String(pot4);
/*
  Serial.print(pot1);
  Serial.print("  ");
  Serial.print(pot2);
  Serial.print("  ");
  Serial.print(pot3);
  Serial.print("  ");
  Serial.println(pot4);
 */
}

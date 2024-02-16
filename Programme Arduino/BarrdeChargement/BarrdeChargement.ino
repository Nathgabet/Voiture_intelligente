#include "DFRobot_GDL.h"
#include <SPI.h>

#define CS_Screen 3
#define DC_Screen 2
#define TR_Screen 4

DFRobot_ST7789_240x240_HW_SPI screen(DC_Screen,CS_Screen,TR_Screen);

uint8_t x=3;
uint16_t y=24;
uint8_t heigth =10;
uint16_t width = 201;

void setup() {

  Serial.begin(9600);
  
  screen.begin();
  
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
 //clean();

}

void loop() {
  
  static int potar1 = 0, potar2 = 0, potar3 = 0, potar4 = 0;
   potar( &potar1, &potar2, &potar3, &potar4);
 
  //menu();
  delay(100);
    
}

void menu(){
  screen.setCursor(25,/*y=*/4 + 33);
  screen.print("Potentiometres");
  screen.setCursor(25,75);
  screen.print("Images");
}

void potar( int *pot1, int *pot2, int *pot3, int *pot4){

  *pot1 = analogRead(A0)/5.12;
  *pot2 = analogRead(A1)/5.12;
  *pot3 = analogRead(A2)/5.12;
  *pot4 = analogRead(A3)/5.12;

  screen.fillRect(x+1, y+1, *pot1, heigth-2, COLOR_RGB565_RED);
  screen.fillRect(x+*pot1+1, y+1 , (width-*pot1-2), heigth-2, COLOR_RGB565_WHITE);

  screen.fillRect(x+1, y+34 , *pot2, heigth-2, COLOR_RGB565_BLUE);
  screen.fillRect(x+*pot2+1, y+34 , (width-*pot2-2), heigth-2, COLOR_RGB565_WHITE);

  screen.fillRect(x+1, y+67 , *pot3, heigth-2, COLOR_RGB565_GREEN );
  screen.fillRect(x+*pot3+1, y+67 , (width-*pot3-2), heigth-2, COLOR_RGB565_WHITE);

  screen.fillRect(x+1, y+100 , *pot4, heigth-2, COLOR_RGB565_ORANGE);
  screen.fillRect(x+*pot4+1, y+100 , (width-*pot4-2), heigth-2, COLOR_RGB565_WHITE);
  
}

void convert(int potar1, int potar2, int potar3,int potar4){

  String str_out ;

  String str_pot1 = String(potar1);
  if (potar1 < 100){
    str_pot1 = "0" + str_pot1;
  }

  String str_pot2 = String(potar2);
  if (potar2 < 100){
    str_pot2 = "0" + str_pot2;
  }

  String str_pot3 = String(potar3);
  if (potar3 < 100){
    str_pot3 = "0" + str_pot3;
  }
  String str_pot4 = String(potar4);
  if (potar4 < 100){
    str_pot4 = "0" + str_pot4;
  }

  str_out = str_pot1 + "," + str_pot2 + "," + str_pot3 + "," + str_pot4 ;

  Serial.print( str_pot1);
  Serial.print("  ");
  Serial.print( str_pot2);
  Serial.print("  ");
  Serial.print( str_pot3);
  Serial.print("  ");
  Serial.print( str_pot4);
  Serial.print("  ");
  Serial.print( str_out);
  Serial.println("  ");

}

void Low(){

  digitalWrite(CS_Screen,LOW);
  digitalWrite(DC_Screen,LOW);

}

void clean(){

  screen.fillScreen(COLOR_RGB565_WHITE);  //font d'ecran une seule couleur
  screen.setTextColor(COLOR_RGB565_BLACK);
  digitalWrite(CS_Screen,LOW);
  digitalWrite(DC_Screen,LOW);

}

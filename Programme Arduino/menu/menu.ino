#include "DFRobot_GDL.h"
#include <SPI.h>

#define CS_Screen 3
#define DC_Screen 2
#define TR_Screen 4


DFRobot_ST7789_240x240_HW_SPI screen(DC_Screen,CS_Screen,TR_Screen);


void setup() {

  Serial.begin(9600);
  
  screen.begin();
  
  screen.fillScreen(COLOR_RGB565_WHITE);  //font d'ecran une seule couleur
  screen.setTextColor(COLOR_RGB565_BLACK);

 //clean();

}

void loop() {
  int statement =1;
  screen.setTextSize(2);

  screen.setCursor(10,/*y=*/37);
  screen.print("Potentiometres");
  
  screen.setCursor(10,77);
  screen.print("Images");
    
  screen.setCursor(10,117);
  screen.print("Test communication");
  
  screen.setCursor(10,157);
  screen.print("Initialisation");
  
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
/*
  if( digitalRead(A4)>255 ){
    statement = statement + 1 ;
  }
  else if ( digitalRead(A)>255 ){
    statement= statement - 1 ;
  }
*/
  Serial.println(analogRead(A4)/2.048);
 // Serial.println(analogRead(A5));
  delay(100);
    
}

void menu(){
  
}

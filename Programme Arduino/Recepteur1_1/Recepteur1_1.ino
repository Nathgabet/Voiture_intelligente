#include "SPI.h"
#include "nRF24L01.h"
#include "RF24.h"

const uint64_t pipe = 0xF0F1F2F3F4LL;
RF24 radio(9,53); // pour MEGA2560 radio(9,53);

void setup() {
 
  Serial.begin(9600);
  radio.begin();
  radio.setChannel(0); // sélectionner le canal radio (0 à 127)

  // vitesse: RF24_250KBPS, RF24_1MBPS ou RF24_2MBPS
  radio.setDataRate(RF24_1MBPS);
  // puissance: RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM
  radio.setPALevel(RF24_PA_HIGH);

  radio.openReadingPipe(1, pipe);
  radio.startListening();
}

void loop() {
  int data;
  
  if (radio.available())
  {
    radio.read(&data, sizeof(data));
    Serial.print("data: ");
    Serial.println(data);

  }
}
#include <SPI.h>
#include "RF24.h"
#include <printf.h>

/*
  ____  _____ ____ _____ _____     _______
  |  _ \| ____/ ___| ____|_ _\ \   / / ____|
  | |_) |  _|| |   |  _|  | | \ \ / /|  _|
  |  _ <| |__| |___| |___ | |  \ V / | |___
  |_| \_\_____\____|_____|___|  \_/  |_____|

*/

RF24 radio(7, 8);

// Topology
//uint64_t senderAddress = 0xF0F0F0F066;
uint64_t livingRoomAddress = 0xF0F0F0F0AA;
uint64_t saunaAddress = 0xF0F0F0F0AB;
uint64_t fireplaceAddress = 0xF0F0F0F0AC;
uint64_t heaterRoomAddress = 0xF0F0F0F0AD;

struct dataStruct {
  float value;
  char unit[8];
  char address[32];
} sensorData;


void setup() {
  Serial.begin(9600);
  
  // Setup and configure radio
  radio.begin();

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  
  radio.openReadingPipe(1, livingRoomAddress);
  radio.openReadingPipe(2, saunaAddress);
  radio.openReadingPipe(3, fireplaceAddress);
  radio.openReadingPipe(4, heaterRoomAddress);

  radio.startListening();
}

void loop(void) {
  byte pipeNo;

  while ( radio.available(&pipeNo)) {
    radio.read(&sensorData, sizeof(sensorData));
    Serial.print("{");
    Serial.print("\"value\":");
    Serial.print(sensorData.value);
    Serial.print(", \"unit\": \"");
    Serial.print(sensorData.unit);
    Serial.print("\", \"address\": \"");
    Serial.print(sensorData.address);
    Serial.println("\"}");
  }
}

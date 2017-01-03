/*
   Dec 2014 - TMRh20 - Updated
   Derived from examples by J. Coliz <maniacbug@ymail.com>
*/
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

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
/**********************************************************/

// Topology
//uint64_t senderAddress = 0xF0F0F0F066;
uint64_t livingRoomAddress = 0xF0F0F0F0AA;
uint64_t saunaAddress = 0xF0F0F0F0AB;
uint64_t fireplaceAddress = 0xF0F0F0F0AC;
uint64_t heaterRoomAddress = 0xF0F0F0F0AD;

/**
  Create a data structure for transmitting and receiving data
  This allows many variables to be easily sent and received in a single transmission
  See http://www.cplusplus.com/doc/tutorial/structures/
*/
struct dataStruct {
  float pressure;
  float humidity;
  float temperature;
  char serial[12];
} sensorData;


void setup() {
  Serial.begin(9600);
  printf_begin();

  //Serial.println(F("Radiotest Receiver starting"));

  // Setup and configure radio
  radio.begin();

  radio.setPALevel(RF24_PA_LOW);

  radio.openReadingPipe(1, livingRoomAddress);
  radio.openReadingPipe(2, saunaAddress);
  radio.openReadingPipe(3, fireplaceAddress);
  radio.openReadingPipe(4, heaterRoomAddress);

  radio.startListening();                       // Start listening

  //radio.printDetails();
}
void loop(void) {
  /****************** Pong Back Role ***************************/
  byte pipeNo;                          // Declare variables for the pipe and the byte received

  while ( radio.available(&pipeNo)) {            // Read all available payloads
    radio.read(&sensorData, sizeof(sensorData));
    //Serial.println(sensorData.temperature);
    Serial.print("{");
    Serial.print("\"temperature\":");
    Serial.print(sensorData.temperature);
    Serial.print(", \"pressure\":");
    Serial.print(sensorData.pressure);
    Serial.print(", \"humidity\":");
    Serial.print(sensorData.humidity);
    Serial.print(", \"sensor\": \"");
    Serial.print(sensorData.serial);
    Serial.println("\"}");
  }
}


/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor
  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650
  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.
  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!
  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "RF24.h"
#include <printf.h>
#include <LowPower.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

#define POWER 8 //power pin to reduce consumption while sleeping

Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
/**********************************************************/

// Topology
//uint64_t senderAddress = 0xF0F0F0F066;
uint64_t myAddress = 0xF0F0F0F0AA;

/**
* Create a data structure for transmitting and receiving data
* This allows many variables to be easily sent and received in a single transmission
* See http://www.cplusplus.com/doc/tutorial/structures/
*/
struct dataStruct{
  float pressure;
  float humidity;
  float temperature;
  char serial[12];
} sensorData;

//ISR(WDT_vect) { Sleepy::watchdogEvent(); }

int sleepMultiplier = 150; // 150 * 8 = (1200seconds) 20 min
int rounds = sleepMultiplier + 1;

void setup() {
  //Serial.begin(9600);
  //printf_begin();
  pinMode(POWER, OUTPUT);  

  //Serial.println(F("BME280 test"));
  while (!bme.begin()) {
    //Serial.println("Could not find a valid BME280 sensor, check wiring!");
    delay(1000);
  }
  strncpy( sensorData.serial, "livingroom", sizeof(sensorData.serial)-1 );
}

void loop() {
  digitalWrite(POWER, HIGH);

  rounds = 0;
  float temperature = bme.readTemperature();
  float pressure = bme.readPressure() / 100.0F;
  float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  float humidity = bme.readHumidity();

  sensorData.pressure = pressure;
  sensorData.humidity = humidity;
  sensorData.temperature = temperature;

  restartRadio();

  radio.write(&sensorData, sizeof(sensorData));

  stopRadio();
  for(int i = 0; i < sleepMultiplier; ++i) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
}

void stopRadio(){
  radio.powerDown();
  /*
  pinMode(13, LOW);
  pinMode(12, LOW);
  pinMode(11, LOW);
  pinMode(10, LOW);
  pinMode(9, LOW);
  */
}

void restartRadio(){
  // Setup and configure radio
  radio.begin();

  radio.setPALevel(RF24_PA_LOW);
  //radio.enableAckPayload();                     // Allow optional ack payloads
  //radio.enableDynamicPayloads();                // Ack payloads are dynamic payloads

  radio.openWritingPipe(myAddress);        // Both radios listen on the same pipes by default, but opposite addresses
  //radio.openReadingPipe(1, senderAddress);     // Open a reading pipe on address 0, pipe 1
  //radio.startListening();                       // Start listening
  //radio.writeAckPayload(1, &counter, 1);        // Pre-load an ack-paylod into the FIFO buffer for pipe 1
  //radio.printDetails();
}


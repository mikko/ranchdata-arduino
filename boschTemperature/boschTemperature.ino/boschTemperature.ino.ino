
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


// BME logic based on https://learn.sparkfun.com/tutorials/sparkfun-bme280-breakout-hookup-guide

#include <Wire.h>
#include <SPI.h>

#include "SparkFunBME280.h"

#include "RF24.h"
#include <printf.h>
#include <LowPower.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define POWER 8 //power pin to reduce consumption while sleeping

BME280 bme;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);
/**********************************************************/

// Network topology
//uint64_t senderAddress = 0xF0F0F0F066;
uint64_t myAddress = 0xF0F0F0F0AA;

/**
* Create a data structure for transmitting and receiving data
* This allows many variables to be easily sent and received in a single transmission
* See http://www.cplusplus.com/doc/tutorial/structures/
*/
struct dataStruct {
  float value;
  char unit[8];
  char address[32];
} sensorData;


int sleepMultiplier = 150; // 150 * 8 = (1200seconds) 20 min

void setup() {
  //Serial.begin(9600);
  //printf_begin();
  pinMode(POWER, OUTPUT);  
  
  strncpy( sensorData.address, "lroom/", sizeof(sensorData.address)-1 );

   //***Driver settings********************************//
  //commInterface can be I2C_MODE or SPI_MODE
  //specify chipSelectPin using arduino pin names
  //specify I2C address.  Can be 0x77(default) or 0x76

  //For I2C, enable the following and disable the SPI section
  bme.settings.commInterface = I2C_MODE;
  bme.settings.I2CAddress = 0x76;

  //For SPI enable the following and dissable the I2C section
  //bme.settings.commInterface = SPI_MODE;
  //bme.settings.chipSelectPin = 10;


  //***Operation settings*****************************//

  //runMode can be:
  //  0, Sleep mode
  //  1 or 2, Forced mode
  //  3, Normal mode
  bme.settings.runMode = 1; //Forced mode

  //tStandby can be:
  //  0, 0.5ms
  //  1, 62.5ms
  //  2, 125ms
  //  3, 250ms
  //  4, 500ms
  //  5, 1000ms
  //  6, 10ms
  //  7, 20ms
  bme.settings.tStandby = 0;

  //filter can be off or number of FIR coefficients to use:
  //  0, filter off
  //  1, coefficients = 2
  //  2, coefficients = 4
  //  3, coefficients = 8
  //  4, coefficients = 16
  bme.settings.filter = 0;

  //tempOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  bme.settings.tempOverSample = 1;

  //pressOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  bme.settings.pressOverSample = 1;

  //humidOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  bme.settings.humidOverSample = 1;
  delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.         
  
  //Calling .begin() causes the settings to be loaded
  bme.begin();
  
}

void loop() {
  digitalWrite(POWER, HIGH);

  float temperature = bme.readTempC();
  float pressure = bme.readFloatPressure() / 100; // Pa to hPa conversion
  float altitude = bme.readFloatAltitudeMeters();
  float humidity = bme.readFloatHumidity();

  digitalWrite(POWER, LOW);
  
  restartRadio();

  // Air pressure  
  strncpy( sensorData.address, "lroom/pressure", sizeof(sensorData.address)-1 );
  sensorData.value = pressure;
  strncpy( sensorData.unit, "hPa", sizeof(sensorData.unit)-1 );

  radio.write(&sensorData, sizeof(sensorData));

  // Humidity
  strncpy( sensorData.address, "lroom/humidity", sizeof(sensorData.address)-1 );
  sensorData.value = humidity;
  strncpy( sensorData.unit, "%", sizeof(sensorData.unit)-1 );

  radio.write(&sensorData, sizeof(sensorData));

  // Temperature
  strncpy( sensorData.address, "lroom/temp", sizeof(sensorData.address)-1 );
  sensorData.value = temperature;
  strncpy( sensorData.unit, "degC", sizeof(sensorData.unit)-1 );

  radio.write(&sensorData, sizeof(sensorData));

  stopRadio();  
  
  for(int i = 0; i < sleepMultiplier; ++i) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
  
  bme.begin(); // Forced mode requires begin to be called before measurement
}

void stopRadio(){
  radio.powerDown();
}

void restartRadio(){
  // Setup and configure radio
  radio.begin();

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);

  radio.openWritingPipe(myAddress);        // Both radios listen on the same pipes by default, but opposite addresses
}


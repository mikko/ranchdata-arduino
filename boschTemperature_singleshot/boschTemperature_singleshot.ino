
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

#define SEALEVELPRESSURE_HPA (1013.25)

#define POWER 8

BME280 bme;

RF24 radio(7, 8);

uint64_t myAddress = 0xF0F0F0F0AA;

struct dataStruct{
  float pressure;
  float humidity;
  float temperature;
  char serial[12];
} sensorData;


int sleepMultiplier = 1200; // 150 * 8 = (1200seconds) 20 min

void setup() {
  pinMode(POWER, OUTPUT);  
  
  strncpy( sensorData.serial, "livingroom", sizeof(sensorData.serial)-1 );

  bme.settings.commInterface = I2C_MODE;
  bme.settings.I2CAddress = 0x76;

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

  rounds = 0;
  float temperature = bme.readTempC();
  float pressure = bme.readFloatPressure() / 100; // Pa to hPa conversion
  float altitude = bme.readFloatAltitudeMeters();
  float humidity = bme.readFloatHumidity();

  digitalWrite(POWER, LOW);

  sensorData.pressure = pressure;
  sensorData.humidity = humidity;
  sensorData.temperature = temperature;

  restartRadio();

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
  radio.openWritingPipe(myAddress);  
}


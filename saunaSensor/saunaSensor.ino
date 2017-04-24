// Radio
#include <SPI.h>
#include "RF24.h"
#include <printf.h>

// Dallas temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>

// Bosch temperature sensor
#include <Wire.h>
#include "SparkFunBME280.h"

// Power save
#include <LowPower.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

BME280 bme;

RF24 radio(7, 8);

// Topology
// uint64_t senderAddress = 0xF0F0F0F0AA;
uint64_t myAddress = 0xF0F0F0F0AB;

struct dataStruct {
  float value;
  char unit[8];
  char address[32];
} sensorData;

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2
DallasTemperature sensors(&oneWire);

int passiveSleepMultiplier = 150; // 150 * 8 = (1200seconds) 20 min
int activeSleepMultiplier = 37.5; // 37.5 * 8 = (300seconds) 5 min
bool isActive = false;
float saunaTemperature = 0;
float activeLimit = 25; // 30;

void setup(void)
{
  Serial.begin(9600);
  
  bme.settings.commInterface = I2C_MODE;
  bme.settings.I2CAddress = 0x76;
  bme.settings.runMode = 1; //Forced mode
  bme.settings.tStandby = 0; // 0.5ms
  bme.settings.filter = 0; // FIR off
  bme.settings.tempOverSample = 1; // *1 oversampling
  bme.settings.pressOverSample = 1; // * 1
  bme.settings.humidOverSample = 1;
  delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.         
  
  //Calling .begin() causes the settings to be loaded
  bme.begin();
  
  sensors.begin();
  strncpy( sensorData.unit, "degC", sizeof(sensorData.unit)-1 );
  isActive = false;
  restartRadio();
}

void loop(void)
{ 
  // Send the command to get temperatures
  sensors.requestTemperatures();  
  saunaTemperature = sensors.getTempCByIndex(0);
  Serial.println(saunaTemperature);
  float temperature = bme.readTempC();
  float pressure = bme.readFloatPressure() / 100; // Pa to hPa conversion
  float altitude = bme.readFloatAltitudeMeters();
  float humidity = bme.readFloatHumidity();
  
  strncpy( sensorData.address, "sauna/temperature", sizeof(sensorData.address) - 1 );
  sensorData.value = saunaTemperature;
  strncpy( sensorData.unit, "degC", sizeof(sensorData.unit)-1 );

  restartRadio();
  
  radio.write(&sensorData, sizeof(sensorData));

  strncpy( sensorData.address, "sauna/active", sizeof(sensorData.address) - 1 );
  strncpy( sensorData.unit, "", sizeof(sensorData.unit)-1 );
  
  sensorData.value = isActive ? 1 : 0;
  radio.write(&sensorData, sizeof(sensorData));

  // Air pressure  
  strncpy( sensorData.address, "utilroom/pressure", sizeof(sensorData.address)-1 );
  strncpy( sensorData.unit, "hPa", sizeof(sensorData.unit)-1 );
  sensorData.value = pressure;

  radio.write(&sensorData, sizeof(sensorData));

  // Humidity
  strncpy( sensorData.address, "utilroom/humidity", sizeof(sensorData.address)-1 );
  strncpy( sensorData.unit, "%", sizeof(sensorData.unit)-1 );
  sensorData.value = humidity;
  
  radio.write(&sensorData, sizeof(sensorData));

  // Temperature
  strncpy( sensorData.address, "utilroom/temp", sizeof(sensorData.address)-1 );
  strncpy( sensorData.unit, "degC", sizeof(sensorData.unit)-1 );
  sensorData.value = temperature;
  
  radio.write(&sensorData, sizeof(sensorData));

  stopRadio();

  if (isActive) {
    for(int i = 0; i < activeSleepMultiplier; ++i) {
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }
  }
  else {
    for(int i = 0; i < passiveSleepMultiplier; ++i) {
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }
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
  radio.openWritingPipe(myAddress);
}


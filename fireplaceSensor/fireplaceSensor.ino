#include "max6675.h"
#include <SPI.h>

int thermoDO = 12;
int thermoCS = 8;
int thermoCLK = 13;

MAX6675 thermocouple;
  
void setup() {
  Serial.begin(9600);
  SPI.begin();

  thermocouple.begin(thermoCS);

  Serial.println("MAX6675 test");
  // wait for MAX chip to stabilize
  delay(500);
}

void loop() {
  float temp = thermocouple.readCelsius();
  Serial.println(temp); 
  delay(10000);
}

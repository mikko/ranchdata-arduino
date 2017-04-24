#include <LowPower.h>

// the setup routine runs once when you press reset:
void setup() {
  
}

// the loop routine runs over and over again forever:
void loop() {
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}

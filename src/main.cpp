#include <Arduino.h>

void setup() {
  // put your setup code here, to run once:
  DDRC = 0b1111111;
  DDRB = 0b000000;

}

void loop() {

  PORTC = 0b0000001;
  delay(1000);
  // put your main code here, to run repeatedly:
}
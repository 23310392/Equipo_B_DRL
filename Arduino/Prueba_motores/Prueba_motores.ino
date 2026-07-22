/* Bldc Motor Speed Controller Using Arduino UNO And ESC and Potentiometer (ElectroiQ Lab)*/
#include <Servo.h>

Servo ESC; 
int Speed; 

void setup() {
  ESC.attach(9,1000,2000);   // attach ESC to pin 9 with 1000-2000µs pulse range
}

void loop() {
  Speed = analogRead(A0);                // read potentiometer (0-1023)
  Speed = map(Speed, 0, 1023, 0, 180);   // map to 0–180 (Servo library scale)
  ESC.write(Speed);                      // write value to ESC
}
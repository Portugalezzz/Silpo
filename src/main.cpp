#include "Arduino.h"
const int dirPin = 2;
const int stepPin = 3;
const int dirPin1 = 4;
const int stepPin1 = 5;
const int stepsPerRevolution = 400;

void setup()
{
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
    pinMode(stepPin1, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  digitalWrite(dirPin, HIGH);
digitalWrite(dirPin1, LOW);
  

}
void loop()
{
    digitalWrite(stepPin, HIGH);
        digitalWrite(stepPin1, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
        digitalWrite(stepPin1, LOW);
    delayMicroseconds(500);
  

  
 
}
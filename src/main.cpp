//#include <Arduino.h>
#include <Arduino.h>
const int dirPin = D0;
const int stepPin = D1;
const int dirPin1 = D3;
const int stepPin1 = D5;
const int stepsPerRevolution = 400;

void setup()
{
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
    pinMode(stepPin1, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  digitalWrite(dirPin, LOW);
digitalWrite(dirPin1, HIGH);
Serial.begin(9600);
  

}
void loop()
{
    digitalWrite(stepPin, HIGH);
        digitalWrite(stepPin1, HIGH);
    delayMicroseconds(200);
    //delay(1);
    digitalWrite(stepPin, LOW);
        digitalWrite(stepPin1, LOW);
    delayMicroseconds(600);
   //delay(1);
   // Установка вращения по часовой стрелки

  //Serial.println("Hello Computer");

 
}
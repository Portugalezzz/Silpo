

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include <AccelStepper.h>



#ifndef APSSID
#define APSSID "ESP_Car"
#define APPSK  "123456789"
#endif

const char *ssid = APSSID;
const char *password = APPSK;




const int dirPin = D0;
const int stepPin = D1;
const int dirPin1 = D3;
const int stepPin1 = D5;

int speed = 400;
int oneMove = 200;
bool straight = false;
bool moving = true;

AccelStepper Rstepper(1, stepPin, dirPin);
AccelStepper Lstepper(1, stepPin1, dirPin1);

ESP8266WebServer server(80);

#define LED 2
int statusLED = HIGH; // Начальный статус светодиода ВЫКЛЮЧЕН. Светодиод инверсный

const String HtmlHtml = "<html><head><meta charset=\"utf-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" /></head>";
const String HtmlTitle = "<h1>Управление машинкой</h1><br/>\n";
const String HtmlIRStateHigh = "<big>ИК Светодиод <b>Отсутствует</b></big><br/><br/>\n"
"<big>Расстояние <b>0.0 м</b></big><br/><br/>\n"
"<big>Температура <b>0.0 С</b></big><br/><br/>\n";
const String HtmlIRStateLOW = "<big>Светодиод <b>Выключен</b></big><br/><br/>\n";
const String HtmlButtons = 
"<a href=\"Move\"><button style=\"background-color:blue;color:white;width:15%;height:10%;font-size:24px;\">&#8657;</button></a>"
"<a href=\"Straight\"><button style=\"background-color:green;color:black;width:15%;height:10%;font-size:24px;\">&#9650;</button></a><br/>"
"<a href=\"Left\"><button style=\"background-color:green;color:black;width:15%;height:10%;font-size:24px;\">&#9668;</button></a>"
"<a href=\"Stop\"><button style=\"background-color:red;color:white;width:15%;height:10%;font-size:24px;\">&#8855;</button></a>"
"<a href=\"Right\"><button style=\"background-color:green;color:black;width:15%;height:10%;font-size:24px;\">&#9658;</button></a><br/>"
"<a href=\"Reverse\"><button style=\"background-color:blue;color:white;width:15%;height:10%;font-size:24px;\">&#8659;</button></a>"
"<a href=\"Back\"><button style=\"background-color:green;color:black;width:15%;height:10%;font-size:24px;\">&#9660;</button></a><br/><br/><br/>"
"<a href=\"Slower\"><button style=\"background-color:blue;color:white;width:22%;height:10%;font-size:24px;\">&#8722;</button></a>"
"<a href=\"Faster\"><button style=\"background-color:blue;color:white;width:22%;height:10%;font-size:24px;\">&#8853;</button></a><br/";

const String HtmlHtmlClose = "</html>";

void response(){
  String htmlRes = HtmlHtml + HtmlTitle;
  if(statusLED == LOW){
  htmlRes += HtmlIRStateLOW;
  }
  else{
  htmlRes += HtmlIRStateHigh;
  }
 
  htmlRes += HtmlButtons;
 htmlRes += HtmlHtmlClose; 
  server.send(200, "text/html", htmlRes);
}

void handleLedOn() {
  statusLED = HIGH; // conditions to make the LED turn on
  digitalWrite(LED, statusLED);
  Serial.println("Светодиод >Выключен");
 response();
}

void handleLedOff() {
  statusLED = LOW; // conditions to make the LED turn off
  digitalWrite(LED, statusLED);
 Serial.println("Светодиод Включен");
 response();
}


void handleStraight() {
      Serial.println("Вперед");
      Lstepper.setSpeed(400);
      Rstepper.setSpeed(-400);
      for (int i = 0; i < oneMove; i++)
        {
        //  Serial.print("И равно ");
        //  Serial.println(i);
          Rstepper.runSpeed();
          Lstepper.runSpeed();
          delay(1);
        }
     // Lstepper.targetPosition();
     // Rstepper.targetPosition();    
      //Lstepper.move(100);
      //Rstepper.move(-100);
      //Rstepper.runSpeed();
      //Lstepper.runSpeed();
      //straight = true;
      //Serial.println(straight);


      response();
}

void handleBack() {
    Lstepper.setSpeed(speed*-1);
    Rstepper.setSpeed(speed);
    for (int i = 0; i < oneMove; i++)
      {
      //  Serial.print("И равно ");
      //  Serial.println(i);
        Rstepper.runSpeed();
        Lstepper.runSpeed();
        delay(1);
      }
 response();
}

void handleLeft() {
      Lstepper.setSpeed(speed);
      Rstepper.setSpeed(speed);
      for (int i = 0; i < oneMove; i++)
        {
        //  Serial.print("И равно ");
        //  Serial.println(i);
          Rstepper.runSpeed();
          Lstepper.runSpeed();
          delay(1);
        }
 response();
}

void handleStop() {
      //Lstepper.setSpeed(400);
      //Rstepper.setSpeed(-400);
    Rstepper.stop ();
    Lstepper.stop ();
    moving = false;
 response();
}

void handlemove() {
  Lstepper.setSpeed(speed);
  Rstepper.setSpeed(speed*-1);
  moving = true;
 response();
}

void handleReverse() {
  Lstepper.setSpeed(speed * -1);
  Rstepper.setSpeed(speed);
  moving = true;
  response();
}

void handleFaster() {
  if (speed < 800)
  {
    speed += 50; 
  }
  
  
  response();
}


void handleSlower() {
  if (speed > 0)
  {
    speed -= 50; 
  }
  response();
}

void handleRight() {
      Lstepper.setSpeed(-400);
      Rstepper.setSpeed(-400);
      for (int i = 0; i < oneMove; i++)
        {
        //  Serial.print("И равно ");
        //  Serial.println(i);
          Rstepper.runSpeed();
          Lstepper.runSpeed();
          delay(1);
        }
 response();
}
 
void setup() {
    delay(1000); 
    WiFi.softAP(ssid, password); 
    IPAddress apip = WiFi.softAPIP();     
    server.on("/", response); 
    server.on("/LEDOff", handleLedOn);
    server.on("/LEDOn", handleLedOff); 

    server.on("/Straight", handleStraight); 
    server.on("/Left", handleLeft); 
    server.on("/Stop", handleStop); 
    server.on("/Right", handleRight); 
    server.on("/Back", handleBack); 
    server.on("/Move", handlemove); 
    server.on("/Reverse", handleReverse); 
    server.on("/Faster", handleFaster); 
    server.on("/Slower", handleSlower); 

    Serial.begin(9600);
    server.begin();    
    pinMode(LED, OUTPUT);
    digitalWrite(LED, statusLED);


    Rstepper.setMaxSpeed(800);
    Rstepper.setSpeed(speed * -1);
    Lstepper.setMaxSpeed(800);
    Lstepper.setSpeed(speed);



}
 
void loop() {
    server.handleClient();
   // Serial.println(straight);
    
    if (moving)
    {
      Rstepper.runSpeed();
      Lstepper.runSpeed();
    }
    
    

  
    
    if (straight)
    {
      for (int i = 0; i < 180; i++)
        {
        //  Serial.print("И равно ");
        //  Serial.println(i);
          Rstepper.runSpeed();
          Lstepper.runSpeed();
          delay(1);
        }
      straight = false;
      //Serial.println(straight);
      //response();
    }
    


    //Rstepper.runSpeed();
    //Lstepper.runSpeed();
}





/*



#include <Arduino.h>
#include <AccelStepper.h>
const int dirPin = D0;
const int stepPin = D1;
const int dirPin1 = D3;
const int stepPin1 = D5;

AccelStepper Rstepper(1, stepPin, dirPin);
AccelStepper Lstepper(1, stepPin1, dirPin1);



void setup()
  {
    Rstepper.setMaxSpeed(400);
    Rstepper.setSpeed(-400);
    Lstepper.setMaxSpeed(400);
    Lstepper.setSpeed(400);
  }

void loop()
 {
    Rstepper.runSpeed();
    Lstepper.runSpeed();
 }



*/




/*
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
 */

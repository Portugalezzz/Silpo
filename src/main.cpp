

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <IRremoteESP8266.h>

#include <AccelStepper.h>
#include <Ultrasonic.h>
#include <EEPROM.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <IRsend.h>

#ifndef APSSID
#define APSSID "ESP_Car"
#define APPSK  "123456789"
#endif



const uint16_t kRecvPin = 0;

IRrecv irrecv(kRecvPin);

decode_results results;

#define IR_LED 4  // ESP8266 GPIO pin to use. Recommended: 4 (D2).

IRsend irsend(IR_LED);  // Set the GPIO to be used to sending the message.
irparams_t filter;
 
uint8_t sRepeats = 1;

uint16_t IRMessageSamsung[71] = 
{
  9000, 4500, 650, 550, 650, 1650, 600, 550, 650, 550,
  600, 1650, 650, 550, 600, 1650, 650, 1650, 650, 1650,
  600, 550, 650, 1650, 650, 1650, 650, 550, 600, 1650,
  650, 1650, 650, 550, 650, 550, 650, 1650, 650, 550,
  650, 550, 650, 550, 600, 550, 650, 550, 650, 550,
  650, 1650, 600, 550, 650, 1650, 650, 1650, 650, 1650,
  650, 1650, 650, 1650, 650, 1650, 600
};


int khz = 38;
uint16_t irSignal1[67] = 
{
  9000, 4500, 650, 550, 650, 1650, 600, 550, 650, 550,
  600, 1650, 650, 550, 600, 1650, 650, 1650, 650, 1650,
  600, 550, 650, 1650, 650, 1650, 650, 550, 600, 1650,
  650, 1650, 650, 550, 650, 550, 650, 1650, 650, 550,
  650, 550, 650, 550, 600, 550, 650, 550, 650, 550,
  650, 1650, 600, 550, 650, 1650, 650, 1650, 650, 1650,
  650, 1650, 650, 1650, 650, 1650, 600
};



const char *ssid = APSSID;
const char *password = APPSK;


Ultrasonic ultrasonic1(17, 16);


uint16_t address = 0x10;
uint16_t  command =  0x34; 




float dist_3[3] = {0.0, 0.0, 0.0};   // массив для хранения трёх последних измерений
float middle, dist, dist_filtered;
float USWall = 30;
unsigned long USWallTimer = 0;
bool obstacle = false;
bool stopped = true;
//bool Moved;


float k;
byte i, delta;
unsigned long dispIsrTimer, sensTimer;



unsigned long randomizedReadingTime = 0;
unsigned long strobTimer = 0;
unsigned long recievingTimer;
int recieveCounter = 0;
int confirmedMessages = 0;



int Htime;       // целочисленная переменная для хранения времени высокого логического уровня
int Ltime;       // целочисленная переменная для хранения времени низкого логического уровня
float Ttime;     // переменная для хранения общей длительности периода
float frequency; // переменная для хранения частоты


//const int dirPin = 10;
//const int stepPin = 0;
//const int dirPin1 = 11;
//const int stepPin1 = 9;

/*
int RECV_PIN = 4;
IRrecv irrecv(RECV_PIN);
decode_results results;

byte SEND_PIN = 14;

IRsend irsend(SEND_PIN);
unsigned long infraRedCode = 0xE0E1488F;

*/

// Dynamic variables for EEPROM
int itterationNumber = 0;
int confirmedMessagesTrigger = 3;
int recieveCounterTrigger = 10;
int speed = 2000; //double
int lowSpeed = 1500; //double
int wall = 200; //float
int recievingTimerFreqmS = 200; //unsigned long 200
int strobTimerTrigger = 20; //unsigned long 100

bool sended = false;

int oneMove = 200;

bool resumed = false;
bool straight = false;
bool moving = true;
bool isServer = true;
bool speedChanged = false;
bool IRCheck = true;
bool newEEPROMData = false;
//AccelStepper Rstepper(1, stepPin, dirPin);
//AccelStepper Lstepper(1, stepPin1, dirPin1);

WebServer server(80);

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
//"<a href=\"Straight\"><button style=\"background-color:green;color:black;width:15%;height:10%;font-size:24px;\">&#9650;</button></a><br/>"
//"<a href=\"Left\"><button style=\"background-color:green;color:black;width:15%;height:10%;font-size:24px;\">&#9668;</button></a>"
//"<a href=\"Stop\"><button style=\"background-color:red;color:white;width:15%;height:10%;font-size:24px;\">&#8855;</button></a>"
//"<a href=\"Right\"><button style=\"background-color:green;color:black;width:15%;height:10%;font-size:24px;\">&#9658;</button></a><br/>"
//"<a href=\"Reverse\"><button style=\"background-color:blue;color:white;width:15%;height:10%;font-size:24px;\">&#8659;</button></a>"
//"<a href=\"Back\"><button style=\"background-color:green;color:black;width:15%;height:10%;font-size:24px;\">&#9660;</button></a><br/><br/><br/>"
//"<a href=\"Slower\"><button style=\"background-color:blue;color:white;width:22%;height:10%;font-size:24px;\">&#8722;</button></a>"
"<a href=\"Faster\"><button style=\"background-color:blue;color:white;width:22%;height:10%;font-size:24px;\">&#8853;</button></a><br/";

const String HtmlHtmlClose = "</html>";


void EEPROMSave()
{
  EEPROM.put(0, ++itterationNumber);
  EEPROM.put(1, confirmedMessagesTrigger);
  EEPROM.put(2, recieveCounterTrigger);
  EEPROM.put(3, speed);
  EEPROM.put(4, lowSpeed); 
  EEPROM.put(5, wall);
  EEPROM.put(6, confirmedMessagesTrigger);
 
  EEPROM.commit();
}

void EEPROMRead()
{
  itterationNumber = EEPROM.read(0);
  confirmedMessagesTrigger = EEPROM.read(1);
  recieveCounterTrigger = EEPROM.read(2);
  speed = EEPROM.read(3);
  lowSpeed = EEPROM.read(4);
  wall = EEPROM.read(5);
  confirmedMessagesTrigger = EEPROM.read(6);
}


void response()
{
  String htmlRes = HtmlHtml + HtmlTitle;
  if(statusLED == LOW)
  {
    htmlRes += HtmlIRStateLOW;
  }
  else
  {
    htmlRes += HtmlIRStateHigh;
  }
 
  htmlRes += HtmlButtons;
  htmlRes += HtmlHtmlClose; 
  server.send(200, "text/html", htmlRes);
}

void handleLedOn() 
{
  statusLED = HIGH; // conditions to make the LED turn on
  digitalWrite(LED, statusLED);
  Serial.println("Светодиод >Выключен");
 response();
}

void handleLedOff() 
{
  statusLED = LOW; // conditions to make the LED turn off
  digitalWrite(LED, statusLED);
 Serial.println("Светодиод Включен");
 response();
}


void handleStraight() 
{
  Serial.println("Вперед");
  //Lstepper.setSpeed(400);
  //Rstepper.setSpeed(-400);
  for (int i = 0; i < oneMove; i++)
  {
  //  Serial.print("И равно ");
  //  Serial.println(i);
    //Rstepper.runSpeed();
    //Lstepper.runSpeed();
    delay(1);
  }

  response();
}

void handleBack() 
{
  //Lstepper.setSpeed(speed*-1);
  //Rstepper.setSpeed(speed);
  for (int i = 0; i < oneMove; i++)
    {
    //  Serial.print("И равно ");
    //  Serial.println(i);
      //Rstepper.runSpeed();
      //Lstepper.runSpeed();
      delay(1);
    }
  response();
}

void handleLeft() 
{
  //Lstepper.setSpeed(speed);
  //Rstepper.setSpeed(speed);
  for (int i = 0; i < oneMove; i++)
    {
    //  Serial.print("И равно ");
    //  Serial.println(i);
      //Rstepper.runSpeed();
      //Lstepper.runSpeed();
      delay(1);
    }
 response();
}

void handleStop() 
{
    //Lstepper.setSpeed(400);
    //Rstepper.setSpeed(-400);
  //Rstepper.stop ();
  //Lstepper.stop ();
  moving = false;
  response();
}

void handlemove() 
{
  //Lstepper.setSpeed(speed);
  //Rstepper.setSpeed(speed*-1);
  moving = true;
  response();
}

void handleReverse() 
{
  //Lstepper.setSpeed(speed * -1);
  //Rstepper.setSpeed(speed);
  moving = true;
  response();
}

void handleFaster() 
{
  if (speed < 800)
  {
    speed += 50; 
  }
  
  
  response();
}


void handleSlower() 
{
  if (speed > 0)
  {
    speed -= 50; 
  }
  response();
}

void handleRight() 
{
  //Lstepper.setSpeed(-400);
  //Rstepper.setSpeed(-400);
  for (int i = 0; i < oneMove; i++)
    {
    //  Serial.print("И равно ");
    //  Serial.println(i);
      //Rstepper.runSpeed();
      //Lstepper.runSpeed();
      delay(1);
    }
 response();
}


// медианный фильтр из 3ёх значений
float middle_of_3(float a, float b, float c) {
  if ((a <= b) && (a <= c)) {
    middle = (b <= c) ? b : c;
  }
  else {
    if ((b <= a) && (b <= c)) {
      middle = (a <= c) ? a : c;
    }
    else {
      middle = (a <= b) ? a : b;
    }
  }
  return middle;
}
 
void setup() 
{
  delay(2000); 
  /*
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
*/
  Serial.begin(9600);
  //server.begin();    
  pinMode(LED, OUTPUT);
  digitalWrite(LED, statusLED);


  //Rstepper.setMaxSpeed(1000);
  //Rstepper.setSpeed(speed * -1);
  //Lstepper.setMaxSpeed(1000);
  //Lstepper.setSpeed(speed);

  pinMode(19, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW); //left wheel
  digitalWrite(19, HIGH);


 //Generator
  ledcSetup(0, speed, 13);
  ledcAttachPin(18, 0);
  
 irrecv.enableIRIn();
 irsend.begin();

 address = random(1000);

}
 
void loop() 
{
  // server.handleClient();
  // Serial.println(straight);
  //ledcWrite(0, 20);
//}
 /*   
 if(!IRCheck) 
  {
    ledcWrite(0, 20);
    //Serial.println("Stop");
    if(speedChanged) 
    {
      ledcSetup(0, speed, 13);
      speedChanged = false;
    }

  }
  
  else if (dist_filtered<wall)
  {
    ledcWrite(0, 0);
    
    //Serial.println("Moving");
  }


  else if ((dist_filtered < (wall+100)) && !speedChanged)
  {
    ledcSetup(0, lowSpeed, 13);
    speedChanged = true;
    Serial.print("Low Speed");
    ledcWrite(0, 20);
  }
  
  
  else if ((dist_filtered > (wall+100)) && speedChanged)
  {

      ledcSetup(0, speed, 13);
      speedChanged = false;
      //Serial.println("Normal Speed");
      ledcWrite(0, 20);
  } 
  
  else
  {
    ledcWrite(0, 20);
  }
   
*/


  if (millis() - sensTimer > 500) 
  {                          // измерение и вывод каждые 50 мс
    // счётчик от 0 до 2
    // каждую итерацию таймера i последовательно принимает значения 0, 1, 2, и так по кругу
    if (i > 1) i = 0;
    else i++;

    //dist_3[i] = (float)ultrasonic1.read() / 57.5;     
    dist_3[i] = (float)ultrasonic1.read();               // получить расстояние в текущую ячейку массива
    //if (!digitalRead(buttPIN)) dist_3[i] += case_offset;    // если включен переключатель стороны измерения, прибавить case_offset
    dist = middle_of_3(dist_3[0], dist_3[1], dist_3[2]);    // фильтровать медианным фильтром из 3ёх последних измерений

    delta = abs(dist_filtered - dist);                      // расчёт изменения с предыдущим
    if (delta > 1) k = 0.7;                                 // если большое - резкий коэффициент
    else k = 0.1;                                           // если маленькое - плавный коэффициент

    dist_filtered = dist * k + dist_filtered * (1 - k);     // фильтр "бегущее среднее"

    
    sensTimer = millis();                                   

    Serial.print("Lenth: ");
    Serial.println(dist_filtered);


    


 
  }
/*
  // Sending IR code
  if ((millis() - strobTimer > strobTimerTrigger) && !sended) 
  //if((millis() - strobTimer > 20) && !sended) 
  {
     // irsend.sendGC(IRMessageSamsung, 71);




      uint32_t dataToSend = irsend.encodeNEC(address, command);
      irsend.sendNEC(dataToSend);

        //--b;

    //  Serial.println("Sending....");
      //irsend.sendElitescreens(157,8,1);
      //irsend.sendRaw(irSignal1,  67, 38); //Note the approach used to automatically calculate the size of the array.
      //irsend.sendNEC(0x00FFE01FUL);
     // strobTimer = millis();
     sended = true;


  }
  if ((millis() - strobTimer > (strobTimerTrigger+20)) && !resumed) 
  {
      irrecv.resume(); 
      resumed = true;

      randomizedReadingTime = (strobTimerTrigger*40) + random(300);
     // Serial.print("Gap = ");
     // Serial.println(randomizedReadingTime); 
  }
  */

/*

  //Recieve and processing IR codes
  //if (millis() - recievingTimer > recievingTimerFreqmS) 
  if ((millis() - strobTimer) > randomizedReadingTime) 
  {
    sRepeats++;
    ++recieveCounter;

   // if (irrecv.decode(&results, nullptr, 5)) 
    if (irrecv.decode(&results)) 
    {

      if(results.address != 0) 
      
      {
        Serial.println("IR OK");
        Serial.println(results.address, HEX);

        //IRCheck = true;
        ++confirmedMessages;
        //results.value = 0;
      }
      else 
      {
        //IRCheck = false;
        Serial.println("Another code   :   ");
        Serial.println(results.address, HEX);

        //--b;
        //results.value = 0;

      }
          
      //irrecv.resume(); 
    }
    else 
    {
      //IRCheck = false;
      //--b;
      Serial.println("Nothing");
    
    }

    if(recieveCounter>=recieveCounterTrigger)
    {
      recieveCounter=0;
      Serial.print("confirmedMessages: ");
      Serial.println(confirmedMessages);
      if (confirmedMessages>confirmedMessagesTrigger) 
      {
        IRCheck = true;
        Serial.println("IRCheck = true;");
      }
      else
      {
        IRCheck = false;
        Serial.println("IRCheck = false;");  
      }

      confirmedMessages = 0;
    }
    irrecv.resume(); 
   // recievingTimer = millis();
   strobTimer = millis();
   sended = false;
   resumed = false;
  }

*/

  if (dist_filtered < USWall)
  {
    
    if (!obstacle)
    {
      USWallTimer = millis();
      obstacle = true;
      Serial.println("Start timer");
    }

    
  }

  else
  {
    //Serial.println("!!!!!");
    obstacle = false;
    if(stopped) 
    {
      ledcWrite(0, 20);
      Serial.println("Moved");
      stopped = false;
    }

  }


  if (((millis() - USWallTimer)>1300) && obstacle)
  {
    if(!stopped)
    {
      ledcWrite(0, 0);
      stopped = true;
      Serial.println("Stopped");
    }

  }

  
}

/*
    irsend.sendNEC(infraRedCode, 32);


      if (irrecv.decode(&results)) 
      {
      Serial.print(results.value, HEX);
      Serial.print(" = ");
      Serial.println(results.value, DEC);
      irrecv.resume();
      }

      */
    // частотомер
    //  Htime=pulseIn(8,HIGH);    // прочитать время высокого логического уровня
    //  Ltime=pulseIn(8,LOW);     // прочитать время низкого логического уровня
        
     // Ttime = Htime+Ltime;
    
     // frequency=1000000/Ttime;  // получение частоты из Ttime в микросекундах

      //Serial.print("Frequency: ");
     // Serial.println(frequency);
     // Serial.println(Htime);
     // Serial.println(Ltime);
    

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

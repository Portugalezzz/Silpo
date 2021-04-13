

//#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

#include <AccelStepper.h>



#ifndef APSSID
#define APSSID "ESP_Car"
#define APPSK  "123456789"
#endif

const char *ssid = APSSID;
const char *password = APPSK;


//#define ECHO 3
//#define TRIG 21

// крутая библиотека сонара
//#include <NewPing.h>
//NewPing sonar(TRIG, ECHO, 400);
#include <Ultrasonic.h>

Ultrasonic ultrasonic1(17, 16);

float dist_3[3] = {0.0, 0.0, 0.0};   // массив для хранения трёх последних измерений
float middle, dist, dist_filtered;
float k;
byte i, delta;
unsigned long dispIsrTimer, sensTimer;
float wall = 50.0;


int Htime;       // целочисленная переменная для хранения времени высокого логического уровня
int Ltime;       // целочисленная переменная для хранения времени низкого логического уровня
float Ttime;     // переменная для хранения общей длительности периода
float frequency; // переменная для хранения частоты


const int dirPin = 10;
const int stepPin = 0;
const int dirPin1 = 11;
const int stepPin1 = 9;

int speed = 1500;
int oneMove = 200;
bool straight = false;
bool moving = true;
bool speedChanged = false;
AccelStepper Rstepper(1, stepPin, dirPin);
AccelStepper Lstepper(1, stepPin1, dirPin1);

WebServer server(8080);

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


    Rstepper.setMaxSpeed(1000);
    Rstepper.setSpeed(speed * -1);
    Lstepper.setMaxSpeed(1000);
    Lstepper.setSpeed(speed);

    pinMode(19, OUTPUT);
    pinMode(5, OUTPUT);
    digitalWrite(5, LOW); //left wheel
    digitalWrite(19, HIGH);

  ledcSetup(0, 3000, 13);
  // подключим канал к GPIO, который нужно контролировать
  ledcAttachPin(18, 0);


}
 
void loop() {
  //  server.handleClient();
   // Serial.println(straight);
    
    if (moving && (dist_filtered>wall))
      {
      // Rstepper.runSpeed();
      // Lstepper.runSpeed();
      ledcWrite(0, 20);
      }
    else ledcWrite(0, 0);
    //     Rstepper.runSpeed();
   //   Lstepper.runSpeed();

  if ((dist_filtered < (wall+50)) && !speedChanged)
    {
      ledcSetup(0, speed/2, 13);
      speedChanged = true;
    }
  else if ((dist_filtered > (wall+50)) && speedChanged)
  {
    ledcSetup(0, speed, 13);
    speedChanged = false;
  }
   
  //delay(1);
 /*   
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
    
*/

    //Rstepper.runSpeed();
    //Lstepper.runSpeed();


  if (millis() - sensTimer > 500) {                          // измерение и вывод каждые 50 мс
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

    //disp.clear();                                           // очистить дисплей
    //disp.float_dot(dist_filtered, 1);                       // вывести
    sensTimer = millis();                                   // сбросить таймер

    Serial.print("Lenth: ");
    Serial.println(dist_filtered);
    Serial.println(ultrasonic1.read());

    // частотомер
    //  Htime=pulseIn(8,HIGH);    // прочитать время высокого логического уровня
    //  Ltime=pulseIn(8,LOW);     // прочитать время низкого логического уровня
        
     // Ttime = Htime+Ltime;
    
     // frequency=1000000/Ttime;  // получение частоты из Ttime в микросекундах

      //Serial.print("Frequency: ");
     // Serial.println(frequency);
     // Serial.println(Htime);
     // Serial.println(Ltime);
    
  }

  //if (micros() - dispIsrTimer > 300) {       // таймер динамической индикации (по-русски: КОСТЫЛЬ!)
    //disp.timerIsr();                         // "пнуть" дисплей
  //  dispIsrTimer = micros();                 // сбросить таймер
  //}

  
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

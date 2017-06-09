#include <MsTimer2.h>
#include <Servo.h>
#include <util/delay.h>

//서보 모터 : + 전원 :red, - 전원 : black, signal : yellow/white

Servo myservo2;




const trig = 12;
const echo = 13; //초음파 센서 trig, echo핀은 각각 12, 13핀에
const INA = 10; //팬 모터 INA 11번 핀
const INB = 11; //INB 10번 핀


unsigned int distance;
unsigned int fanOn = 0; //조건 맞으면 1, 아니면 0
unsigned int pos = 0; // variable to store the servo position
unsigned int a;
unsigned int temperature;

char inChar;
char inData[10];
char numb[5];
byte index = 0;
boolean systemStatus = false;

void condition();
void trigger();
void angle_plus();
void angle_minus();

void setup() {

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(INA, OUTPUT);
  pinMode(INB, OUTPUT);
  Serial.begin(9600);

  MsTimer2::set(100, trigger);
  MsTimer2::start();

}


void loop() {
  // put your main code here, to run repeatedly:



  for (pos = 45; pos <= 135; pos += 1) { // goes from 15 degrees to 165 degrees
    angle_plus();

  }

  for (pos = 135; pos >= 45; pos -= 1) { // goes from 165 degrees to 15 degrees
    angle_minus();                    // waits 15ms for the servo to reach the position

  }




}


void condition() { //선풍기 켜질 조건 맞으면 1, 아니면 0, 온도 센서 추가 필요

  index = 0;

  if (Serial.available()) {

    delay(20);
    while (Serial.available())

    {
      inData[index++] = Serial.read();

    }
    inData[index] = '\0';

    for (int i = 0 ; i < 5; i++)
    {
      numb[i] = inData[i + 5];
    }
    temperature = atoi(numb);
    Serial.println(inData);
  }

  if (strncmp(inData, "wind", 4) == 0)
  {
    systemStatus =  !systemStatus;
    inData[0] = 0;
  }

  if (temperature < 24)
    systemStatus = false;
  else if(temperature >= 24 && systemStatus)
    systemStatus = true;



  if (distance < 30 && systemStatus ) //거리가 30cm이하
  {
    fanOn = 1;
    digitalWrite(INA, LOW);
    digitalWrite(INB, HIGH);
  }

  else if (distance > 30 || !systemStatus)
  {
    fanOn = 0;
    digitalWrite(INA, LOW);
    digitalWrite(INB, LOW);
  }
}

void trigger() //거리 triggering 하는 함수
{
  digitalWrite(trig, HIGH);
  _delay_ms(2);
  digitalWrite(trig, LOW);

  distance = pulseIn(echo, HIGH, 10000) * 17 / 1000; //cm로 표시
  if (distance == 0)
    distance = 77;

  condition();
}

void angle_plus() { //각도 더하는 함수

  myservo2.write(pos);
  delay(15);

  if (fanOn == 1 || !systemStatus)
  {
    a = pos;  //FANON인 상태에서 각도 멈추게 해줌
    myservo2.detach();
    pos = a - 1; //FANON인 상태에서 각도 멈추게 해줌
  }

  else if (fanOn == 0 && systemStatus)
  {
    myservo2.attach(9);
    myservo2.write(pos);
    delay(15);
  }

}
void angle_minus() {

  myservo2.write(pos);
  delay(15);

  if (fanOn == 1 || !systemStatus)
  {
    a = pos; //FANON인 상태에서 각도 멈추게 해줌
    myservo2.detach();
    pos = a + 1; //FANON인 상태에서 각도 멈추게 해줌
  }

  else if (fanOn == 0 && systemStatus)
  {
    myservo2.attach(9);
    myservo2.write(pos);
    delay(15);
  }

}


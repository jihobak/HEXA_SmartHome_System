#include <MsTimer2.h>
#include <SoftwareSerial.h>
#include <Servo.h>

Servo myservo; //식기를 회전시킬 서보모터
SoftwareSerial mySerial(10, 11); //(RX, TX)_와이파이와 연결된 아두이노와의 Serial 통신 설정

int hour = 21; //현재의 시, 분, 초를 입력
int minute = 31;
int sec = 50;


char inChar;
char inData[10];
byte index = 0;

void servo();
void increment();
void check(); 


void setup() {
  Serial.begin(57600);//PC와의 Serial 통신 속도 설정

  MsTimer2::set(1000,increment);//1s마다 increment 함수를 호출하도록 설정
  MsTimer2::start();//위에서 설정한 동작을 시작

  while (!Serial) {
    ;
    }//컴퓨터와 통신이 될 때까지 기다림

  while(!mySerial) {
    ;
   }//아두이노와 통신이 될 때까지 기다림
    


  Serial.println("connected with PC & arduino_Serial");

  mySerial.begin(4800); //아두이노와의 Serial 통신 속도 설정
  mySerial.write("Feed_Servo is connected with your arduino");
}















void loop() { 
  index = 0;
  inData[0] = 0; 

  if (mySerial.available()) //mySerial에 입력값이 있다면
  {
    delay(20);
  while(mySerial.available())//mySerial의 입력을 inData 문자열에 저장함
    {
      inData[index++] = mySerial.read(); 
      
    }
    inData[index] = '\0';//inData문자열의 끝에 널문자를 설정
    Serial.println(inData);
    check(); //inData에 저장된 문자열이 명령어와 일치하는지 확인
  }
 
 
  
   else if(hour == 21 && minute ==32)//배식기 동작 시간을 설정
  {
    if(sec == 0)
    {delay(1000);
    servo();}
    

    else
    return;
  }

 
 else if(hour==9&&minute==30)
 {
  if(sec==0)
  {delay(1000);
    servo();}
    

    else
    return;
  
 }

  else
  myservo.detach();



}


void servo()//배식기의 서보모터를 동작시키는 함수
{
    myservo.attach(8);
    myservo.write(45);
    delay(180);
    myservo.detach();
    

   


}

void increment()//시, 분, 초를 갱신하는 함수
{
  sec++;

  minute += sec / 60;
  sec %= 60;

  hour += minute / 60;
  minute %= 60;

  hour %= 24;

}

void check() //inData문자열과 명령어가 일치하는지 확인하는 함수
{
 int turn = strncmp(inData,"feed",4);//inData에 저장된 문자열과 "feed"를 비교
 if (turn == 0)//같다면 
 {
 Serial.print(turn);
 Serial.print("servo");
 servo(); 서보모터를 동작시킴
 
 }

 turn = 1;//서보모터를 한번만 동작시키도록 turn값 재설정
}


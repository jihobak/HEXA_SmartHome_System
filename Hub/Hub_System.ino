#include <DHT11.h>
#include <MsTimer2.h>
#include <SoftwareSerial.h>
#define DEBUG false

// Cloud update period
#define PERIOD 300000

//Device No.
#define LED 0
#define WIND 1
#define FEED 2

// << Pin number >>
#define LEDPIN 10
#define DHTPIN 6 // Pin which is connected to the DHT sensor
DHT11 dht11(DHTPIN);


/* [Arduino] | [ESP8266]
   RX: pin2  |    TX
   TX: pin3  |    RX
*/
SoftwareSerial wind_board(2,3);
SoftwareSerial feed_board(8,9);

boolean LED_STATE = false;
boolean WIND_STATE = false;
boolean FEED_STATE = false;
float TEMP = 0;
float HUMIDITY = 0;

// Server
String cloud = "hexa-smarthome-168905.appspot.com";

// WiFi parameters to be configured
//const char* ssid = "iPhone";//"OpenX";
//const char* password = "01077594655";//"yes_openx";
String ssid = "\"OpenX\",";
String password = "\"yes_openx\"";

String sendData(String command, unsigned long timeout, boolean debug);
void make_response(int connection_id, String rsp);
void make_get_request(String host, int port);
void get_data();
void cloud_upload();

unsigned long prev_time = 0;

void setup(void)
{ 
  // Setting baud rate
  Serial.begin(9600);
  wind_board.begin(9600);
  feed_board.begin(9600);


  // LED pin setting and initialization
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);

  sendData("AT+RST\r\n", 1000, DEBUG); // reset module
  sendData("AT+CWMODE=3\r\n", 1000, DEBUG); // configure as AP+STA
  sendData("AT+CWLAP\r\n", 5000, DEBUG); // list of available AP
  sendData("AT+CWJAP="+ssid+password+"\r\n", 10000, DEBUG); // join the access point
  
  sendData("AT+CIFSR\r\n", 2000, DEBUG); // get ip address
  sendData("AT+CIPMUX=1\r\n", 1000, DEBUG); // configure for multiple connections
  sendData("AT+CIPSERVER=1,81\r\n", 1000, DEBUG); // turn on server on port 81
  
}

void loop()
{
  int sensor; // sensor no.

  //*********************************************
  //****Part1. handling request
  //****
  if(Serial.available()) 
  { 
    // check if the esp is sending a message
    if(Serial.find("+IPD,")) 
    {
      delay(500); // wait for the serial buffer to fill up (read all the serial data)
      
      // get the connection id so that we can then disconnect
      int connectionId = Serial.read()-48; // subtract 48 because the read() function returns 
                                           // the ASCII decimal value and 0 (the first decimal number) starts at 48

      //sensor
      // 0: LED
      // 1: WIND
      // 2: FEED
      if(Serial.find("sensor=")) // advance cursor to "led="
      {
        sensor = Serial.read()-48; 
        if(sensor == LED)
        {
          //Serial.print("LED_STATE>>>");
          //Serial.println(LED_STATE);
          LED_STATE = !LED_STATE;
          digitalWrite(LEDPIN, LED_STATE); // toggle pin

          // make reponse
          make_response(connectionId, "<html><h1>LED</h1></html>");
        } 
        else if(sensor == WIND)
        {
          wind_board.listen();
          wind_board.print(String("wind,")+String(TEMP,2));
          make_response(connectionId, "<html><h1>WIND</h1></html>");
          wind_board.end();
        }
        else if(sensor == FEED)
        {
          feed_board.listen();
          feed_board.print("feed");
          make_response(connectionId, "<html><h1>FEED</h1></html>");
          feed_board.end();
        }
        else
        {
          make_response(connectionId, "<html><h1>WTF</h1></html>");
        }
      }
      else
      {
        
      }

      // make close command
      String closeCommand = "AT+CIPCLOSE="; 
      closeCommand+=connectionId; // append connection id
      closeCommand+="\r\n";
      sendData(closeCommand,1000,DEBUG); // close connection
    }
  }
  //*********************************************

  //*********************************************
  //****Part2. SensorData upload and send
  //****
  // every 5mins upload sensor data to Cloud.
  unsigned long current_time = millis();
  if(current_time - prev_time > PERIOD) {
    cloud_upload();
    prev_time = current_time;
  }  

}


String sendData(String command, unsigned long timeout, boolean debug)
{
  String response = "";


  Serial.print(command); // send the read character to the esp8266
  
  unsigned long time = millis();

  // Read data during period.
  while((time+timeout) > millis())
  {
    while(Serial.available())
    {
      // The esp has data
      char c = Serial.read();
      response +=c;     
    }
  }
  
  if(debug)
  {
    wind_board.println(response);
  }
  
  return response;
}


void make_response(int connection_id, String rsp)
{
  String response = "";

  // response header
  response +="HTTP/1.1 200 OK\n\n\r";
  response += rsp;
  response +="\r\n";
  
  String cmd = "AT+CIPSEND=";
  cmd += connection_id;
  cmd += ",";
  cmd += response.length();
  cmd += "\r\n";

  sendData(cmd, 1000, DEBUG);
  sendData(response, 1000, DEBUG);
}

void make_get_request(String host, int port)
{
  // Our clould adrress is "hexa-smarthome-168905.appspot.com";
  String adress_for_at = "";
  String at_cmd = "";
  String at_cmd2 = "";
  String get_request = "";

  adress_for_at += "\"";
  adress_for_at += host;
  adress_for_at += "\"";


  get_request +="GET /sensor?";
  get_request +="temp";
  get_request +="=";
  get_request +=String(TEMP,2);
  get_request +="&";
  get_request +="humd";
  get_request +="=";
  get_request +=String(HUMIDITY,2);
  
  get_request +=" ";
  get_request +="HTTP/1.1\r\n";
  get_request +="HOST: ";
  get_request += host;
  get_request +="\r\n\r\n";

  
  // Connect to Cloud(AT Command)
  at_cmd += "AT+CIPSTART=";
  at_cmd += "4,\"TCP\"";
  at_cmd += ",";
  at_cmd += adress_for_at;
  at_cmd += ",";
  at_cmd += String(port);
  at_cmd += "\r\n";

  // Specify number of bytes you'll send
  at_cmd2 += "AT+CIPSEND=";
  at_cmd2 += "4,";
  at_cmd2 += get_request.length();
  at_cmd2 += "\r\n";

  sendData(at_cmd, 1000, DEBUG);
  sendData(at_cmd2, 1000, DEBUG);
  sendData(get_request, 1000, DEBUG);
}


void get_data()
{
  int err;

  if((err=dht11.read(HUMIDITY, TEMP))==0)
  {
    
  }
  else
  {
    
  }
}

void cloud_upload()
{
  
  get_data(); // Update sensor data
  wind_board.print(String("data,")+String(TEMP,2)); // send temp data to wind_board
  make_get_request(cloud, 80); // send sensor data to cloud
}



/*
  Code for the Controller and Servo example:
  The position of the servo can be controlled with a Controller. 
  The data of the controller can also be used to navigate an AR drone.
  The sensordata is sent to a MQTT Server:
  Servo1 = analogue y-Axis Data of controller from 0-180 
  Servo2 = analogue x-Axis Data of controller from 0-180 
  links,rechts,oben,unten,mittig = digital Data of controller
  V 1.0
  
  Created by Martin Schubert and Armin Fischer, Sept 2022.
  School: HTBLuVA Wiener Neustadt
  E-Mail: sum@htlwrn.ac.at and fia@htlwrn.ac.at
*/
//********************************************************************************************************************************************************
//******************************************************* LIBRARIES **************************************************************************************
//********************************************************************************************************************************************************
#include <Arduino.h>

#include <FlashStorage.h>
#include <WiFiNINA.h>
#include <utility/wifi_drv.h>
#include <PubSubClient.h>
#include "string.h"
#include <ArduinoJson.h>
#include <Servo.h>


//********************************************************************************************************************************************************
//******************************************************* PINS **************************************************************************************
//********************************************************************************************************************************************************
#define GREEN_LED_pin   25            // ESP32 RGB-Led  - only for statusmessages
#define BLUE_LED_pin    27
#define RED_LED_pin     26
#define PIN_LED          7             // PIN for front led
#define PIN_SETUP        2             // PIN FOR SETTING UP Appkey,Server, SSID and password
#define PIN_SERVO1       3             // PWM for Servo1
#define PIN_SERVO2       4             // PWM for Servo2
#define PIN_JOY_X        PIN_A1         // PIN Joystick axis 1
#define PIN_JOY_Y        PIN_A2         // PIN Joystick axis 2
#define PIN_JOY_BUT      5              // PIN Joystick Button





//********************************************************************************************************************************************************
//******************************************************* VARIABLES AND SETUP FOR SENSORS AND FUNCTIONS **************************************************************************************
//********************************************************************************************************************************************************



//******************************************************* Random Client ID  **************************************
char Client_ID [20];

// ****************************************************** SERVO **************************************************
Servo servo1;
Servo servo2;
#define INITAL_POS_S1 90
#define INITAL_POS_S2 90
int position_servo1;
int position_servo2;


// JOYSTICK

int val_joystick_x;
int val_joystick_y;


//******************************************************* MQTT  **************************************************

#define MQTT_SUB "/cmd/#"     // apended to given topic
#define MQTT_PUB "/data"      // apended to given topic

char mqttData [300];
String property_list[]={"Servo1","Servo2"};
String mqtt_sub;
String mqtt_pub;

//******************************************************* FLASH SETUP  **************************************************
typedef struct {
  boolean valid;
  char Broker [40];
  char Port[10];
  char SSID[20];
  char WifiPWD[20];
  char Topic[60];
  char User [20];
  char Pass [20];
  char Client_ID [20];
} userData;
FlashStorage(flash_store,userData); // Reserve a portion of flash memory to store a userData
userData settings;                  //Create struct to save settings from serial input



//******************************************************* Variable for Internet Connections **************************************************

bool flag=0; //Request if void loop() is run the first time
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

//******************************************************* Variable intervall settings **************************************************

const long LED_intervall = 300;                // sendingLED_intervalll
const long MQTT_intervall = 100;              // sending MQTT-messages intervall

unsigned long LED_previousMillis = 0;
unsigned long MQTT_previousMillis =0;

bool LEDflag = 0;

volatile bool REMOTEflag = 0;                          // indicates remote or local control
                                              // is automatic set to true when MQTT - command is received
                                              // can be reset by Joystick button



//********************************************************************************************************************************************************
// ****************************************************** FUNCTIONS ****************************************************************
//********************************************************************************************************************************************************

//*******************************************************MKR1010 Status LED **************************************************
void setLEDColor(uint8_t R, uint8_t G, uint8_t B);

//******************************************************* handler for incomming mqtt ****************************************
void mqtt_callback(char* topic, byte* payload, unsigned int length);

//
void reconnect();

void remote_off();
// *********************************For Serial-Input
String GetTerminalText();

//***********************************************************************************************************************************************************
//******************************************************* VOID SETUP - SETUP YOUR ARDUINO *******************************************************************
//***********************************************************************************************************************************************************
void setup() {
// RGB-Led
  WiFiDrv::pinMode(RED_LED_pin, OUTPUT);   //RED
  WiFiDrv::pinMode(GREEN_LED_pin, OUTPUT); //GREEN
  WiFiDrv::pinMode(BLUE_LED_pin, OUTPUT);  //BLUE
 
 
// LED control    
  pinMode(PIN_LED, OUTPUT);

// Joystick
  pinMode(PIN_JOY_X, INPUT);
  pinMode(PIN_JOY_Y, INPUT);
  pinMode(PIN_JOY_BUT,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_JOY_BUT) , remote_off ,LOW);

// Servo
  servo1.attach (PIN_SERVO1);
  servo2.attach (PIN_SERVO2);
  position_servo1 = INITAL_POS_S1;
  position_servo2 = INITAL_POS_S2;
  servo1.write (position_servo1);
  servo2.write (position_servo2);


// Setup control
  pinMode(PIN_SETUP, INPUT_PULLUP);
  
  
  Serial.begin(9600);     
    
  settings = flash_store.read();
if ((settings.valid == false) || (digitalRead(PIN_SETUP)==LOW)) {

//    ... ask for user data.
    setLEDColor(255,0,0); // turn LED to RED
    Serial.setTimeout(120000);
    while (!Serial) {}
    Serial.println("***********************************************************");
    Serial.println("*                                                         *");
    Serial.println("* DigiPro  Connection Setup (c) 2022 FIASUM               *");
    Serial.println("*                                                         *");
    Serial.println("* ATTENTION use PUTTY or KITTY                            *");
    Serial.println("* --> to see entered chars turn on local echo and         *");
    Serial.println("*     local line editing in Putty!!!                      *");
    // Serial.println("* --> confirm every value with <ENTER>                    *");
   // Serial.println("* --> backspace and del ar not supported                  *");
    Serial.println("***********************************************************\n\n");    
    Serial.println("* Topic for subscription will get */cmd/#                 *");
    Serial.println("* Topic for publish will get */data                       *");
    Serial.println("* --> Example: Topic entered is edu/iot2021               *");
    Serial.println("*       --> subscription will be edu/iot2021/cmd/#        *");
    Serial.println("*       --> publish will be edu/iot2021/data              *");
    Serial.println("***********************************************************\n\n"); 

  //  #define MQTT_SUB "/cmd/#"
//#define MQTT_PUB "/data"

    Serial.println("Insert SSID:");
    String SSID = GetTerminalText();
    //String SSID = Serial.readStringUntil('\r');
    Serial.println();
    Serial.println("Insert Wifi Password:");
    String wifiPWD = GetTerminalText();
    //String wifiPWD = Serial.readStringUntil('\r');
    Serial.println();
    Serial.println("Insert Broker:");
    String Broker = GetTerminalText();
    //String Broker = Serial.readStringUntil('\r');
    Serial.println();
    Serial.println("Insert Port:");
    String Port = GetTerminalText();
    //String Port = Serial.readStringUntil('\r');
    Serial.println();
    Serial.println("Insert MQTT-Topic:");
    String Topic = GetTerminalText();
    //String Topic = Serial.readStringUntil('\r');
    Serial.println();
    Serial.println("Insert MQTT-User:");
    String User = GetTerminalText();
    //String User = Serial.readStringUntil('\r');
    Serial.println();
    Serial.println("Insert MQTT-Pass:");
    String Pass = GetTerminalText();
    //String Pass = Serial.readStringUntil('\r');
Serial.println();
    Serial.println("Insert MQTT-Client_ID:");
    String Client_ID = GetTerminalText();    
    Serial.println();
    //mqtt_sub = MQTT_SUB;
    //mqtt_sub += Topic;
    //mqtt_pub = Topic + MQTT_PUB;
    // Fill the "settings" structure with the data entered by the user...

    SSID.toCharArray(settings.SSID, SSID.length()+2);                 
    wifiPWD.toCharArray(settings.WifiPWD, wifiPWD.length()+2);
    Broker.toCharArray(settings.Broker, Broker.length()+2);
    Port.toCharArray(settings.Port, Port.length()+2);
    Topic.toCharArray(settings.Topic, Topic.length()+2);
    User.toCharArray(settings.User, User.length()+2);
    Pass.toCharArray(settings.Pass, Pass.length()+2);
    Client_ID.toCharArray(settings.Client_ID, Client_ID.length()+2);

    // set "valid" to true, so the next time we know that we have valid data inside
    settings.valid = true;
    flash_store.write(settings);
    
   
    // Print a confirmation of the data inserted.
    Serial.println();
    Serial.println("Settings stored ! \n");
    delay (1000);


  } 

  delay(2000);
  
  setLEDColor(0,255,0);
  
    Serial.print("SSID: |");
    Serial.print(settings.SSID);
    Serial.println("|");
    Serial.print("WifiPWD: |");
    Serial.print(settings.WifiPWD);
    Serial.println("|");
    Serial.print("Broker: |");
    Serial.print(settings.Broker);
    Serial.println("|");
    Serial.print("Port: |");
    Serial.print(settings.Port);
    Serial.println("|");
    Serial.print("Topic: |");
    Serial.print(settings.Topic);
    Serial.println("|");
    Serial.print("MQTT-User: |");
    Serial.print(settings.User);
    Serial.println("|");
    Serial.print("MQTT-Pass: |");
    Serial.print(settings.Pass);
    Serial.println("|");
    Serial.print("MQTT-Client_ID: |");
    Serial.print(settings.Client_ID);
    Serial.println("|");
  
  // create topics for pub and sub
  mqtt_sub = String(settings.Topic)  + MQTT_SUB;
  mqtt_pub = String(settings.Topic) + MQTT_PUB;
  
  mqttClient.setBufferSize (1024);                // Standard is only 128 !
  mqttClient.setServer(settings.Broker,atoi(settings.Port));
  mqttClient.setCallback(mqtt_callback);
  // reconnect();


//******************************************************* CREATE INTERNET CONNECTION  ******************************************************************* 
      delay(1000);  //Wait 1 sec for module initialization
      //Attempt a WiFi connection to desired access point at ssid, password
     
  

//**************************************************************************************************************************************************
//******************************************************* VOID LOOP - MAIN CODE  *******************************************************************
//**************************************************************************************************************************************************
}

void loop() {
    
    if (flag==0){
      delay(1000);  //Wait 1 sec for module initialization

      //Attempt a WiFi connection to desired access point at ssid, password
      while ( WiFi.status() != WL_CONNECTED) {
        //WiFi.disconnect();
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(settings.SSID);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        WiFi.begin(settings.SSID, settings.WifiPWD);
        delay(10000); //Wait 10 secs for establishing connection
      }
      //Print WiFi status
      //Print SSID name
      Serial.print("SSID: ");
      Serial.println(WiFi.SSID());

      //Print ipv4 assigned to WiFi101 module
      IPAddress ip = WiFi.localIP();
      
      Serial.print("IP Address: ");
      Serial.println(ip);
  
      //Print signal strength for WiFi101 module
      long rssi = WiFi.RSSI();
      Serial.print("Signal strength (RSSI):");
      Serial.print(rssi);
      Serial.println(" dBm");

      if (WiFi.status() == WL_CONNECTED) {
      flag = true;
      } else {
          WiFi.end();
          }
     
      } else if (flag && WiFi.status() != WL_CONNECTED) {
        WiFi.end();
        flag = false;
    
      
      }
    //delay (2000);
    while (!mqttClient.connected()) {
      mqttClient.disconnect();
        Serial.print("Attempting MQTT connection...");



       if (mqttClient.connect ( settings.Client_ID , settings.User, settings.Pass)) {
 //       if (mqttClient.connect( str , settings.User, settings.Pass)) {

        //if (mqttClient.connect(MQTT_ClientID, settings.User, settings.Pass)) {
          Serial.println("Connected to MQTT-Server!");


          delay (1000);
          //"Message arrived: topic: " + String(topic)
           //mqttClient.subscribe()
          mqttClient.subscribe(mqtt_sub.c_str());
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
      }

  
    mqttClient.loop();
   
   unsigned long currentMillis = millis();
   if (currentMillis - LED_previousMillis >= LED_intervall) {
    // save the last time a message was sent
     LEDflag = !LEDflag;
     LED_previousMillis = currentMillis;
   //setLEDColor(0,255,0); // turn LED to Green
    if (LEDflag) setLEDColor(0,0,255);
    else 
    {
      if (!REMOTEflag) setLEDColor(0,255,0);
      else setLEDColor(255,0,0);
    }  
  }


// map Joystick value to servoposition

  if (!REMOTEflag) {
    val_joystick_x = analogRead(PIN_JOY_X);
    val_joystick_y = analogRead(PIN_JOY_Y);
    position_servo1 =map( val_joystick_x,0,1024,0,180);
    position_servo2 =map( val_joystick_y,0,1024,0,180);
  } 
    servo1.write(position_servo1);
    servo2.write(position_servo2);


//******************************************************* SENDING DATA TO MQTT-Broker  *******************************************************************
    
  currentMillis = millis();
if (currentMillis - MQTT_previousMillis >= MQTT_intervall) {
    
    
    
    float send_values[]={position_servo1,position_servo2};
    int sizearray=sizeof(property_list)/sizeof(String);


  DynamicJsonDocument doc(2048);
  String json;
  for(int i=0;i<sizearray;i++){
    doc[property_list[i]].set(send_values[i]);
  }
  serializeJson(doc, json);
  

    Serial.print("Sending message: ");
    // Serial.println(settings.Topic);
    Serial.println(json);

    // send message, the Print interface can be used to set the message contents

    json.toCharArray(mqttData, json.length()+1);

    setLEDColor(0,0,255);
    mqttClient.publish(mqtt_pub.c_str(),mqttData);
    //setLEDColor(0,255,0);
    MQTT_previousMillis = currentMillis;
  
  
  }
}


//**************************************************************************************************************************************************
//******************************************************* FUNCTION CODE SECTION  *******************************************************************
//**************************************************************************************************************************************************

void mqtt_callback(char* topic, byte* payload, unsigned int length){

 int i = 0;
 // Hilfsvariablen für die Convertierung der Nachricht in ein String
 char message_buff[300];
 
 Serial.println("Message arrived: topic: " + String(topic));
 Serial.println("Length: " + String(length,DEC));
 if (!REMOTEflag) REMOTEflag=1;                                 // switch to remote mode when cmd is received
                                     // convert message to char array
 for(i=0; i<length; i++) {
 message_buff[i] = payload[i];
 }
 message_buff[i] = '\0';
 

  if (strstr(topic,"pos_servo1")){     //Servo1
      position_servo1 = atoi(message_buff);
      // servo1.write(atoi(message_buff));
  }
  if (strstr(topic,"pos_servo2")){     //Servo2
      position_servo2 = atoi(message_buff);
      // servo2.write(atoi(message_buff));
  }
  
}



//******************************************************* SET LED COLOUR  *******************************************************************
void setLEDColor(uint8_t R, uint8_t G, uint8_t B){
  //R, G, and B values should not exceed 255 or be lower than 0.
  //set ESP32 wifi module RGB led color
  WiFiDrv::analogWrite(RED_LED_pin, R); //Red
  WiFiDrv::analogWrite(GREEN_LED_pin, G); //Green
  WiFiDrv::analogWrite(BLUE_LED_pin, B);  //Blue
}

//******************************************************* Enable Editing during serial connection  *******************************************************************
// Thanks to Izaak Martin !
String GetTerminalText() {
  unsigned int RetPressed = 0;
  String InString;
  while (RetPressed == 0) {
    if (Serial.available()) {
      int InCharInt = Serial.read();
      if (InCharInt != 13) { // Return not pressed
        if (InCharInt != 127) { // Backspace not pressed
          char InChar = char(InCharInt);
          Serial.print(InChar);
          InString += InChar;
        } else { // Backspace pressed
          Serial.print(char(127));
          int InStringLength = InString.length();
          InString.remove(InStringLength - 1);
        }
      } else { // Return pressed
        RetPressed = 1;
      }
    }
  }
  return (InString);
}

void remote_off()   // Turn off remotemode
{
  REMOTEflag = 0;

}

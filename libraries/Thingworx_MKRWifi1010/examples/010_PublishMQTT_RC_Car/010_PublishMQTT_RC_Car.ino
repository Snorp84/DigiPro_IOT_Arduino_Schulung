/*
  Code for the DigiPro RC car to read all sensors:
  Temperature, Humidity, RPM, Current, Distance, Gyroscope
  The sensordata is sent to the Thingworx cloud.

  
  Created by Martin Schubert and Armin Fischer, Oct 2020.
  School: HTBLuVA Wiener Neustadt
  E-Mail: sum@htlwrn.ac.at and fia@htlwrn.ac.at
*/
//********************************************************************************************************************************************************
//******************************************************* LIBRARIES **************************************************************************************
//********************************************************************************************************************************************************
#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <SPI.h>
#include <FlashStorage.h>
#include <WiFiNINA.h>
#include <utility/wifi_drv.h>
#include <Thingworx_MKRWifi1010.h>
#include <Thingworx_MKRWifi1010_Variable.h>
#include <ArduinoMqttClient.h>

//********************************************************************************************************************************************************
//******************************************************* PINS **************************************************************************************
//********************************************************************************************************************************************************
#define GREEN_LED_pin   25            // ESP32 RGB-Led
#define BLUE_LED_pin    27
#define RED_LED_pin     26
#define PIN_FAN         3
#define PIN_Temp        6             // DHT22 - temparature/humidity
#define PIN_Current     PIN_A5        // ACS 712 - current 
#define PIN_RPM         0             // BB3609 - ir light barrier
#define RPM_HOLES       20            // number of pulses per rotation
#define PIN_TRIG        4             //ULTRASONIC PINS
#define PIN_ECHO        1             //ULTRASONIC PINS
#define PIN_SETUP       2             //PIN FOR SETTING UP Appkey,Server, SSID and password
#define PIN_LED         7


//********************************************************************************************************************************************************
//******************************************************* VARIABLES AND SETUP FOR SENSORS AND FUNCTIONS **************************************************************************************
//********************************************************************************************************************************************************

//******************************************************* FLASH SETUP  **************************************************
typedef struct {
  boolean valid;
  char Broker [30];
  char Port[10];
  char Topic[50];
  char SSID[30];
  char WifiPWD[30];
} userData;
FlashStorage(flash_store,userData); // Reserve a portion of flash memory to store a userData
userData settings;                  //Create struct to save settings from serial input

//******************************************************* GYROSCOPE **************************************************
const int MPU=0x68;
float AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float pitch,roll;
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
int c =0;

//******************************************************* CURRENT SENSOR **************************************************
float sens_analog_value=0;
float sens_analog_k = 0.0184;
float sens_analog_d = 2122;

//******************************************************* RPM ENCODER **************************************************
volatile unsigned long sens_rpm_t2, sens_rpm_count;
unsigned long sens_rpm_value , sens_rpm_t1;

//******************************************************* TEMPERATURE AND HUMIDITY **************************************************
DHT sens_dht( PIN_Temp  ,DHT22);
float sens_dht_temp;
float sens_dht_hum;

//******************************************************* DISTANCE **************************************************
long duration;
int distance;

//******************************************************* LED **************************************************
boolean Toggle_LED = true;

//******************************************************* Variable for Internet Connections **************************************************
bool flag=0; //Request if void loop() is run the first time
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
const long interval = 500;
unsigned long previousMillis = 0;


//********************************************************************************************************************************************************
// ****************************************************** FUNCTIONS ****************************************************************
//********************************************************************************************************************************************************

//*******************************************************MKR1010 Status LED **************************************************
void setLEDColor(uint8_t R, uint8_t G, uint8_t B);

//******************************************************* TEMPERATURE AND HUMIDITY **************************************************
void temp_hum();

//******************************************************* RPM **************************************************
void sens_rpm_isr();
void rpm();

//******************************************************* GYROSCOPE **************************************************
void gyro();

//******************************************************* Distance **************************************************
void HCSR04();

//******************************************************* Current **************************************************
void current();


//***********************************************************************************************************************************************************
//******************************************************* VOID SETUP - SETUP YOUR ARDUINO *******************************************************************
//***********************************************************************************************************************************************************
void setup() {
// RGB-Led
  WiFiDrv::pinMode(RED_LED_pin, OUTPUT);   //RED
  WiFiDrv::pinMode(GREEN_LED_pin, OUTPUT); //GREEN
  WiFiDrv::pinMode(BLUE_LED_pin, OUTPUT);  //BLUE
 
 //initialize temp sensor    
    sens_dht.begin();

 // currentsensor - anaolg input
    analogReadResolution(12);                         // set to maximum resolution of MKR1010 12bit --> 4096 Steps (standard is 10)
    
 // interrupt for rotary encoder
    pinMode(PIN_RPM, INPUT);
    attachInterrupt ( digitalPinToInterrupt (PIN_RPM), sens_rpm_isr, FALLING);
    sens_rpm_t1=0;
    sens_rpm_t2=0;
    sens_rpm_count=0;
    Wire.begin();
    Wire.beginTransmission(MPU);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
      // Configure Accelerometer Sensitivity - Full Scale Range (default +/- 2g)
    Wire.beginTransmission(MPU);
    Wire.write(0x1C); //Talk to the ACCEL_CONFIG register (1C hex)
    Wire.endTransmission(true);
    Wire.requestFrom(MPU, 1);
    byte x = Wire.read(); //the value of Register-28 is in x
    x = x | 0b00011000;     //appending values of Bit4 and Bit3 --> AFS_SEL=3 (+/- 16g)
    
    Wire.beginTransmission(MPU);
    Wire.write(0x1C);
    Wire.write(x); //(+/- 16g );
    
    Wire.endTransmission(true);

    // Configure Gyro Sensitivity - Full Scale Range (default +/- 250deg/s)
    Wire.beginTransmission(MPU);
    Wire.write(0x1B); // Talk to the GYRO_CONFIG register (1B hex)
    Wire.endTransmission(true);

    Wire.requestFrom(MPU, 1);
    x = Wire.read(); //the value of Register-27 is in x
    x = x | 0b00010000;     //appending values of Bit4 and Bit3 --> FS_SEL = 1 (+/- 500°/sec)
    
    Wire.beginTransmission(MPU);
    Wire.write(0x1B);
    Wire.write(x); //( );
    
    Wire.endTransmission(true);


// initialize HC-SR04    
    pinMode(PIN_TRIG, OUTPUT); // Sets the trigPin as an Output
    pinMode(PIN_ECHO, INPUT); // Sets the echoPin as an Input

// LED control    
  pinMode(PIN_LED, OUTPUT);

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
    Serial.println("* DigiPro RC-Car Connection Setup                         *");
    Serial.println("*                                                         *");
    Serial.println("* ATTENTION - When using PUTTY                            *");
    Serial.println("* --> entered chars are not displayed!!!                  *");
    Serial.println("* --> confirm every value with <CTRL> + <J>               *");
    Serial.println("*                                                         *");
    Serial.println("***********************************************************\n\n");    

    

     Serial.println("Insert Broker:");
    String Broker = Serial.readStringUntil('\n');
    Serial.println("Insert Port:");
    String Port = Serial.readStringUntil('\n');
    Serial.println("Insert Topic:");
    String Topic = Serial.readStringUntil('\n');
    Serial.println("Insert SSID:");
    String SSID = Serial.readStringUntil('\n');
    Serial.println("Insert Wifi Password:");
    String wifiPWD = Serial.readStringUntil('\n');
    
    
    // Fill the "settings" structure with the data entered by the user...

    SSID.toCharArray(settings.SSID, SSID.length()+2);                 
    wifiPWD.toCharArray(settings.WifiPWD, wifiPWD.length()+2);
    Broker.toCharArray(settings.Broker, Broker.length()+2);
    Port.toCharArray(settings.Port, Port.length()+2);
    Topic.toCharArray(settings.Topic, Topic.length()+2);

    // set "valid" to true, so the next time we know that we have valid data inside
    settings.valid = true;
    flash_store.write(settings);
    
   
    // Print a confirmation of the data inserted.
    Serial.println();
    Serial.println("Settings stored ! \n");

    Serial.print("Broker: |");
    Serial.print(settings.Broker);
    Serial.println("|");
    Serial.print("Port: |");
    Serial.print(settings.Port);
    Serial.println("|");
    Serial.print("Topic: |");
    Serial.print(settings.Topic);
    Serial.println("|");
    Serial.print("SSID: |");
    Serial.print(settings.SSID);
    Serial.println("|");
    Serial.print("WifiPWD: |");
    Serial.print(settings.WifiPWD);
    Serial.println("|");

    setLEDColor(0,255,0);

  } else {
     Serial.println("Insert Broker:");
    String Broker = Serial.readStringUntil('\n');
    Serial.println("Insert Port:");
    String Port = Serial.readStringUntil('\n');
    Serial.println("Insert Topic:");
    String Topic = Serial.readStringUntil('\n');
    Serial.println("Insert SSID:");
    String SSID = Serial.readStringUntil('\n');
    Serial.println("Insert Wifi Password:");
    String wifiPWD = Serial.readStringUntil('\n');
    
    
    // Fill the "settings" structure with the data entered by the user...

    SSID.toCharArray(settings.SSID, SSID.length()+2);                 
    wifiPWD.toCharArray(settings.WifiPWD, wifiPWD.length()+2);
    Broker.toCharArray(settings.Broker, Broker.length()+2);
    Port.toCharArray(settings.Port, Port.length()+2);
    Topic.toCharArray(settings.Topic, Topic.length()+2);

    // set "valid" to true, so the next time we know that we have valid data inside
    settings.valid = true;
    flash_store.write(settings);
    
   
    // Print a confirmation of the data inserted.
    Serial.println();
    Serial.println("Settings stored ! \n");

    Serial.print("Broker: |");
    Serial.print(settings.Broker);
    Serial.println("|");
    Serial.print("Port: |");
    Serial.print(settings.Port);
    Serial.println("|");
    Serial.print("Topic: |");
    Serial.print(settings.Topic);
    Serial.println("|");
    Serial.print("SSID: |");
    Serial.print(settings.SSID);
    Serial.println("|");
    Serial.print("WifiPWD: |");
    Serial.print(settings.WifiPWD);
    Serial.println("|");

    setLEDColor(0,255,0);
  
}
}

//**************************************************************************************************************************************************
//******************************************************* VOID LOOP - MAIN CODE  *******************************************************************
//**************************************************************************************************************************************************


void loop() {

 //******************************************************* CREATE INTERNET CONNECTION  ******************************************************************* 
    if (flag==0){
      delay(1000);  //Wait 1 sec for module initialization

      //Attempt a WiFi connection to desired access point at ssid, password
      while ( WiFi.status() != WL_CONNECTED) {
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
    
      Serial.print("Attempting to connect to the MQTT broker: ");
      Serial.println(settings.Broker);

      if (!mqttClient.connect(settings.Broker, atoi(settings.Port))) {
        Serial.print("MQTT connection failed! Error code = ");
        Serial.println(mqttClient.connectError());

       
      }
      else {
      Serial.println("You're connected to the MQTT broker!");
      Serial.println(); 
      }
    

  
     //delay (TPOST); //alle TPOST Sekunden an den Thingworx Server senden, Einstellen in dem Variablen File
//******************************************************* CURRENT  ******************************************************************* 
    current();

//******************************************************* RPM  *******************************************************************
    rpm();

//******************************************************* TEMPERATURE AND HUMIDITY  *******************************************************************
    temp_hum();

//******************************************************* DISTANZ  *******************************************************************
    HCSR04();

//******************************************************* GYROSCOPE  *******************************************************************
    gyro();

//******************************************************* SENDING DATA TO THINGWORX  *******************************************************************
    float send_values[]={sens_dht_temp,sens_dht_hum,sens_rpm_value,sens_analog_value,distance,pitch,roll,AcX,AcY,AcZ,GyX,GyY,GyZ};
    int sizearray=sizeof(property_list)/sizeof(String);

  
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();

  // avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;

  DynamicJsonDocument doc(2048);
  String json;
  for(int i=0;i<sizearray;i++){
    doc[property_list[i]].set(send_values[i]);
  }
  serializeJson(doc, json);
  

    Serial.print("Sending message to topic: ");
    Serial.println(settings.Topic);
    Serial.println(json);

    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(settings.Topic);
    mqttClient.print(json);
    mqttClient.endMessage();
}
}


//**************************************************************************************************************************************************
//******************************************************* FUNCTION CODE SECTION  *******************************************************************
//**************************************************************************************************************************************************

//******************************************************* INTERRUPT FOR RPM  *******************************************************************
void sens_rpm_isr(){               
    sens_rpm_count++;
    sens_rpm_t2 = millis();
}

//******************************************************* RPM  *******************************************************************
void rpm(){
    if (sens_rpm_t2 > sens_rpm_t1) {
        sens_rpm_value = (unsigned)(long)(60000 * sens_rpm_count / (sens_rpm_t2 - sens_rpm_t1))/RPM_HOLES;
        sens_rpm_t1 = sens_rpm_t2;
        sens_rpm_count = 0;
    }
    else {
        sens_rpm_value = 0;
    }
        Serial.println("[HC-020K]");  
        Serial.print(" |           RPM = "); Serial.println(sens_rpm_value); 
        Serial.println();
}

//******************************************************* Current  *******************************************************************
void current(){
    int n;
    float v_value = 0;
    for (n=1;n<=10;n++){
      v_value=v_value+analogRead(PIN_Current);
    }
    v_value=v_value/10;

    sens_analog_value = roundf(((v_value-sens_analog_d)*sens_analog_k)*100)/100 ;
        Serial.println("[ACS712]");
    Serial.print(" |       v_value = "); Serial.println(v_value);
    Serial.print(" |       Current = "); Serial.println(sens_analog_value);
    Serial.println();
}

//******************************************************* TEMPERATURE AND HUMIDITY  *******************************************************************
void temp_hum() {
    sens_dht_hum = sens_dht.readHumidity();
    sens_dht_temp = sens_dht.readTemperature();
    Serial.println("[DHT22]");  
    Serial.print(" |   Temperature = "); Serial.println(sens_dht_temp);
    Serial.print(" |      Humidity = "); Serial.println(sens_dht_hum);
    Serial.println();
}

void HCSR04(){
    // Clears the trigPin
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(PIN_ECHO, HIGH);
    // Calculating the distance
    distance= duration*0.034/2;
    // Prints the distance on the Serial Monitor
    Serial.println("[HCSR04]");
    Serial.print(" |      Distance = ");
    Serial.println(distance);
    Serial.println(" ");
}

//******************************************************* GYROSCOPE  *******************************************************************
void gyro() {
Wire.beginTransmission(MPU);
Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
Wire.endTransmission(false);
Wire.requestFrom(MPU,14,true); // request a total of 14 registers (6 AC; 2 Temp; 6 GY)

//read accel data and compute g-value (AFS_SEL ist set t0 3 in setup!! +/- 16 g )
AcX=(float)(int16_t)(Wire.read()<<8|Wire.read())/2048;                      //Typcasting to float!
AcY=(float)(int16_t)(Wire.read()<<8|Wire.read())/2048;
AcZ=(float)(int16_t)(Wire.read()<<8|Wire.read())/2048;

Wire.read()<<8|Wire.read(); // Not used - Temperature

//read gyro data & apply correction (FS_SEL ist set to 1  +/- 500 °/sec)
GyX=(float)(int16_t)(Wire.read()<<8|Wire.read())/65.5;
GyY=(float)(int16_t)(Wire.read()<<8|Wire.read())/65.5;
GyZ=(float)(int16_t)(Wire.read()<<8|Wire.read())/65.5;

//get pitch/roll
pitch = atan(AcZ/sqrt((AcY*AcY) + (AcX*AcX)));  // different orientation of sensor!
roll = atan(AcY/sqrt((AcX*AcX) + (AcZ*AcZ)));

pitch = pitch * (180.0/3.14);     //convert radians into degrees
roll = roll * (180.0/3.14) ;

//send the data out the serial port
Serial.println("[GY521]-Angle");
Serial.print(" |         Pitch = "); Serial.println(pitch);
Serial.print(" |          Roll = "); Serial.println(roll);
Serial.println("[GY521]-Acc");
Serial.print(" |             X = "); Serial.println(AcX);
Serial.print(" |             Y = "); Serial.println(AcY);
Serial.print(" |             Z = "); Serial.println(AcZ);
Serial.println("[GY521]-Gyro");
Serial.print(" |             X = "); Serial.println(GyX);
Serial.print(" |             Y = "); Serial.println(GyY);
Serial.print(" |             Z = "); Serial.println(GyZ);
Serial.println(" ");
}

//******************************************************* SET LED COLOUR  *******************************************************************
void setLEDColor(uint8_t R, uint8_t G, uint8_t B){
  //R, G, and B values should not exceed 255 or be lower than 0.
  //set ESP32 wifi module RGB led color
  WiFiDrv::analogWrite(RED_LED_pin, R); //Red
  WiFiDrv::analogWrite(GREEN_LED_pin, G); //Green
  WiFiDrv::analogWrite(BLUE_LED_pin, B);  //Blue
}

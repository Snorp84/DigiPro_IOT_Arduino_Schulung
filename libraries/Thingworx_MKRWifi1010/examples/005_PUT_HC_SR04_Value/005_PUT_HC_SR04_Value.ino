/*
  Example for using an Arduino MKR1010 with a PTC Thingworx Server. The Library "Thingworx_MKR1010" is used.
  The distance is measured with a HC-SR04 Ultrasonic Sensor and sent to the Server.
  
PLEASE DEFINE ALL VARIABLES IN THE "Thingworx_MKR1010_Variable.h" FILE
  
  Created by Armin Fischer, Octobre 2019.
  School: HTL Wiener Neustadt
  E-Mail: fia@htlwrn.ac.at
*/

//Definition of used Libraries
#include "Thingworx_MKRWifi1010.h"
#include "Thingworx_MKRWifi1010_Variable.h"

//Definitions of variables for the sensor
const int trigPin = 4;  //Trigger Pin on digital port x
const int echoPin = 5;  //Echo Pin on digital port y
long duration;

float sensorvalue=0;

// Define Thingworx Class (1 per Thing)
ThingWorx myThing(host, port, appKey, thingName);  

void setup() {
  //Definition of used Pins by the Sensor
  pinMode(trigPin, OUTPUT);                       // set the trigger Pin to output
  pinMode(echoPin, INPUT);                        // set the echo Pin to input
  //
  Serial.begin(9600);                              //Serial communications with computer at 9600 bauds for debug purposes
  myThing.Wifi(ssid, password);                    //Start the Wifi Connection

}

void loop() {
  if (millis() - lastConnectionTime > TPOST)      //Send request to server every TPOST seconds
  { 
    //Code for the sensor, the value should be stored in the variable "sensorvalue"
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  sensorvalue= duration*0.034/2;


    //Send data with PUT Request to Thingworx
    myThing.put("HC_SR04",sensorvalue);  //Send distance to server platform
    
    lastConnectionTime = millis();               //Refresh last connection time for if 
  }
}

/*
  Example for using an Arduino MKR1000 with a PTC Thingworx Server. The Library "Thingworx_MKR1000" is used.
  A relay on the Arduino MKR Relay Proto Shield is set to high or low.

PLEASE DEFINE ALL VARIABLES IN THE "Thingworx_MKR1000_Variable.h" FILE
  
  Created by Armin Fischer, Jan 2019.
  School: TGM Vienna
  E-Mail: afischer2@tgm.ac.at
*/

//Definition of used Libraries
#include "Thingworx_MKRWifi1010.h"
#include "Thingworx_MKRWifi1010_Variable.h"

// Define Thingworx Class (1 per Thing)
ThingWorx myThing(host, port, appKey, thingName); 

//Relay 1 is on pin 1, relay 2 is on pin 2
#define RELAY_1 1
#define RELAY_2 2

void setup() {
  Serial.begin(9600);                             //Serial communications with computer at 9600 bauds for debug purposes
  myThing.Wifi(ssid, password);                   //Start the Wifi Connection
  pinMode(RELAY_1,OUTPUT);                        //Digital pin 1 is an output pin
  pinMode(RELAY_2,OUTPUT);                        //Digital pin 2 is an output pin

}

void loop() {
  if (millis() - lastConnectionTime > TPOST)      //Send request to server every TPOST seconds
  { 
    //Logic for the relay
    if( myThing.getjson("Ventilator") == 1.0)
    {
      digitalWrite(RELAY_1,HIGH);   
    }
    else
    {
      digitalWrite(RELAY_1,LOW); 
    }
    
    lastConnectionTime = millis();               //Refresh last connection time for if 
  }
}

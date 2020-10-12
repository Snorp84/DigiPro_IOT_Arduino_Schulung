/*
  Example for using an Arduino MKRWifi1010 with a PTC Thingworx Server. The Library "Thingworx_MKRWifi1010" is used.
  A value from the Thingworx Server is obtained.

PLEASE DEFINE ALL VARIABLES IN THE "Thingworx_MKRWifi1010_Variable.h" FILE
  
  Created by Armin Fischer, Aug 2019.
  School: HTBLuVA Wiener Neustadt
  E-Mail: 
*/

//Definition of used Libraries
#include "Thingworx_MKRWifi1010.h"
#include "Thingworx_MKRWifi1010_Variable.h"

// Define Thingworx Class (1 per Thing)
ThingWorx myThing(host, port, appKey, thingName); 

//Variable for Sensor Values
float sensor_1;

void setup() {
  Serial.begin(9600);                              //Serial communications with computer at 9600 bauds for debug purposes
  myThing.Wifi(ssid, password);                    //Start the Wifi Connection

}

void loop() {
  if (millis() - lastConnectionTime > TPOST)      //Send request to server every TPOST seconds
  { 
    sensor_1=myThing.getjson("Sensorwert_1");     //Get data with GET Request from Thingworx
    
    lastConnectionTime = millis();               //Refresh last connection time for if 
  }
}

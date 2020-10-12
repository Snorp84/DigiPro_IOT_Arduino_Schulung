/*
  Example for using an Arduino MKR1010 with a PTC Thingworx Server. The Library "Thingworx_MKR1010" is used.
  Any Sensorvalue is meassured and sent to the Server.
  
PLEASE DEFINE ALL VARIABLES IN THE "Thingworx_MKR1010_Variable.h" FILE
  
  Created by Armin Fischer, Octobre 2019.
  School: HTL Wiener Neustadt
  E-Mail: fia@htlwrn.ac.at
*/

//Definition of used Libraries
#include "Thingworx_MKRWifi1010.h"
#include "Thingworx_MKRWifi1010_Variable.h"

//Definitions of variables for the sensor

float sensorvalue=0;

// Define Thingworx Class (1 per Thing)
ThingWorx myThing(host, port, appKey, thingName);  

void setup() {
  //Definition of used Pins by the Sensor


  //
  Serial.begin(9600);                              //Serial communications with computer at 9600 bauds for debug purposes
  myThing.Wifi(ssid, password);                    //Start the Wifi Connection

}

void loop() {
  if (millis() - lastConnectionTime > TPOST)      //Send request to server every TPOST seconds
  { 
    //Code for the sensor, the value should be stored in the variable "sensorvalue"



    //Send data with PUT Request to Thingworx
    myThing.put("Sensorvalue_Thingworx",sensorvalue);  //Send distance to server platform
    
    lastConnectionTime = millis();               //Refresh last connection time for if 
  }
}

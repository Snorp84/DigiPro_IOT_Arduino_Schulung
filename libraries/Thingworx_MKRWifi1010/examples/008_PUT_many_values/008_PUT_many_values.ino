/*
  Example for using an Arduino Wifi MKR1010 with a PTC Thingworx Server. The Library "Thingworx_MKRWifi1010" is used.
  The value of variable "sensor_value" is sent to a thing's property.

PLEASE DEFINE ALL VARIABLES IN THE "Thingworx_MKRWifi1010_Variable.h" FILE
  
  Created by Armin Fischer, Aug 2019.
  School: HTBLuVA Wiener Neustadt
  E-Mail: 
*/

//Definition of used Libraries
#include "Thingworx_MKRWifi1010.h"
#include "Thingworx_MKRWifi1010_Variable.h"

// Define Thingworx Class (1 per Thing)
ThingWorx myThing(host, port, appKey, thingName, property_list);  

//Definition variables
float sensor_value=20.5;

void setup() {
  
  Serial.begin(9600);                              //Serial communications with computer at 9600 bauds for debug purposes
  myThing.Wifi(ssid, password);                    //Start the Wifi Connection

}

void loop() {

  float values[]={32.39,33,22,50.995,13.03,123};
  int sizearray=sizeof(property_list)/sizeof(String);
  
  
  if (millis() - lastConnectionTime > TPOST)      //Send request to server every TPOST seconds
  {  
    //myThing.put("BME280_TEMP2",sensor_value); //Send values to server platform
    myThing.put_many(values,sizearray); //Send values to server platform
    lastConnectionTime = millis();               //Refresh last connection time for if 
  }
}

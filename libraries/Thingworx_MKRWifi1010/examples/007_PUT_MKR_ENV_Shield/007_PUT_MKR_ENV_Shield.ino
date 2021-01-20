/*
  Example for using an Arduino MKR1010 with a PTC Thingworx Server. The Library "Thingworx_MKR1010" is used.
  The sensor values of the Arduino MKR Environment shield are meassured and sent to the Server.
  
PLEASE DEFINE ALL VARIABLES IN THE "Thingworx_MKR1010_Variable.h" FILE
  
  Created by Armin Fischer, Decembre 2019.
  School: HTL Wiener Neustadt
  E-Mail: fia@htlwrn.ac.at
*/

//Definition of used Libraries
#include "Thingworx_MKRWifi1010.h"
#include "Thingworx_MKRWifi1010_Variable.h"
#include <Arduino_MKRENV.h>

//Definitions of variables used by the shield
float temperature;
float humidity;
float pressure;
float illuminance;
float uva;
float uvb;
float uvIndex;


// Define Thingworx Class (1 per Thing)
ThingWorx myThing(host, port, appKey, thingName, property_list,ssid,password );  

void setup() {
  Serial.begin(9600);                              //Serial communications with computer at 9600 bauds for debug purposes
  
  //Check if Environment shield is installed/working properly
  if (!ENV.begin()) {
    Serial.println("Failed to initialize MKR ENV shield!");
    while (1);
  }
  
  myThing.Wifi();                    //Start the Wifi Connection

}

void loop() {
  if (millis() - lastConnectionTime > TPOST)      //Send request to server every TPOST seconds
  { 
    //Code for the sensor, the value should be stored in the variable "sensorvalue"
	  temperature = ENV.readTemperature();
 	  humidity    = ENV.readHumidity();
  	pressure    = ENV.readPressure();
  	illuminance = ENV.readIlluminance();
  	uva         = ENV.readUVA();
  	uvb         = ENV.readUVB();
  	uvIndex     = ENV.readUVIndex();

    float send_values[]={temperature,humidity,pressure,illuminance,uva,uvb,uvIndex};
    int sizearray=sizeof(property_list)/sizeof(String);

    myThing.put_many(send_values,sizearray);
    //Send data with PUT Request to Thingworx
//    myThing.put("Temperatur",temperature);
//    myThing.put("Luftfeuchtigkeit",humidity);
//    myThing.put("Luftdruck",pressure);
//    myThing.put("Lichtstaerke",illuminance);
//    myThing.put("UVA",uva);
//    myThing.put("UVB",uvb);
//    myThing.put("UV-Index",uvIndex);
    
    lastConnectionTime = millis();               //Refresh last connection time for if 
  }
}

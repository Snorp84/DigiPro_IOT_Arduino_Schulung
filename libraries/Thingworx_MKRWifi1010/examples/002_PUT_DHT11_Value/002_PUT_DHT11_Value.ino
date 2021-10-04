/*
  Example for using an Arduino MKR1010 with a PTC Thingworx Server. The Library "Thingworx_MKR1010" is used.
  The humidity and the temperature is measured with a DHT11 and sent to the Thingworx Server

PLEASE DEFINE ALL VARIABLES IN THE "Thingworx_MKR1010_Variable.h" FILE
  
  Created by Armin Fischer, Octobre 2019.
  School: HTL Wiener Neustadt
  E-Mail: fia@htlwrn.ac.at
*/

//Definition of used Libraries
#include "Thingworx_MKRWifi1010.h"
#include "Thingworx_MKRWifi1010_Variable.h"
#include <DHT.h>

//Definition for DHT11 sensor
#define DHT11_PIN 3                                 //DHT11 Sensor is on Pin 3
DHT sens_dht( DHT11_PIN  ,DHT11);

// Define Thingworx Class (1 per Thing)
ThingWorx myThing(host, port, appKey, thingName, property_list, ssid, password);  

void setup() {
  pinMode(DHT11_PIN, INPUT);                       // set DHT11_PIN to input
  Serial.begin(9600);                              //Serial communications with computer at 9600 bauds for debug purposes
  myThing.Wifi();                    //Start the Wifi Connection
  sens_dht.begin();

}

void loop() {
  if (millis() - lastConnectionTime > TPOST)      //Send request to server every TPOST seconds
  { 
    //Send data with PUT Request to Thingworx
    myThing.put("DHT11_Temp",sens_dht.readTemperature());  //Send temperature to server platform
    myThing.put("DHT11_Moist",sens_dht.readHumidity());      //Send humidity to server platform
    
    lastConnectionTime = millis();               //Refresh last connection time for if 
  }
}

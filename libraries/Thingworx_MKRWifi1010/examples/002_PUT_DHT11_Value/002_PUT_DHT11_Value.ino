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
#include <dht.h>

//Definition for DHT11 sensor
dht DHT;
#define DHT11_PIN 3                                 //DHT11 Sensor is on Pin 3

// Define Thingworx Class (1 per Thing)
ThingWorx myThing(host, port, appKey, thingName);  

void setup() {
  pinMode(DHT11_PIN, INPUT);                       // set DHT11_PIN to input
  Serial.begin(9600);                              //Serial communications with computer at 9600 bauds for debug purposes
  myThing.Wifi(ssid, password);                    //Start the Wifi Connection

}

void loop() {
  if (millis() - lastConnectionTime > TPOST)      //Send request to server every TPOST seconds
  { 
    //Collect DHT11 Data
    int chk = DHT.read11(DHT11_PIN);
    //Send data with PUT Request to Thingworx
    myThing.put("DHT11_Temp",DHT.temperature);  //Send temperature to server platform
    myThing.put("DHT11_Moist",DHT.humidity);      //Send humidity to server platform
    
    lastConnectionTime = millis();               //Refresh last connection time for if 
  }
}

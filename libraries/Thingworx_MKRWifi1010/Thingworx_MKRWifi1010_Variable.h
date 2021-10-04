/**********************************************************************************************************
 * Thingworx_MKRWifi1010_Variable.h - Definition of variables, which are used with the "Thingworx_MKRWifi1010.h" library
 * Created by Armin Fischer, Oct 2021
 * Version 4.0
 * School: HTBLuVA Wiener Neustadt
 * E-Mail: fia@htlwrn.ac.at
************************************************************************************************************/

#ifndef Thingworx_MKRWifi1010_Variable_H
#define Thingworx_MKRWifi1010_Variable_H

//**********************************************************************************
//**********************************TIMING VARIABLES********************************
//**********************************************************************************
const unsigned long TPOST = 2000;  //Time between requests to TWX server (every 2 sec)
unsigned long lastConnectionTime = 0; //Last connection ms time between server requests

//**********************************************************************************
//**********************************WIFI CONNECTION********************************
//**********************************************************************************

char* ssid = ""; //WiFi SSID
char* password = ""; //WiFi Pass


//**********************************************************************************
//**********************************HOST THINGWORX*******************************
//**********************************************************************************
char* host = "xxxxx.twx.htl.schule";  //TWX Host for HTL Austria twx.htl.schule (without http at beginning)
unsigned int port = 443; //TWX host port for https 

//**********************************************************************************
//**********************************THINGWORX VARIABLES*******************************
//**********************************************************************************
char appKey[] = "";
char thingName[] = "RC-Car";  //Thing name from TWX
String property_list[]={"Temperatur","Feuchtigkeit","Drehzahl","Strom","Distanz","Pitch","Roll","Beschleunigung_x","Beschleunigung_y","Beschleunigung_z","Gyro_x","Gyro_y","Gyro_z"};

#endif

/**********************************************************************************************************
 * Thingworx_MKRWifi1010.h - Library for up- and downloading values to/from a Thingworx "Thing" with an Arduino Wifi MKR1010
 * Created by Armin Fischer, Oct 2020
 * Version 3.0
 * School: HTBLuVA Wiener Neustadt
 * E-Mail: 
 * 
 * The REST API documentation was obtained from:
 * https://support.ptc.com/appserver/cs/view/solution.jsp?n=CS249622&art_lang=en&posno=1&q=Thingworx%20post%20request%20arduino&ProductFamily=ThingWorx%7CNRN%7CAxeda&source=search
************************************************************************************************************/

#ifndef Thingworx_MKRWifi1010_h
#define Thingworx_MKRWifi1010_h

#include "Arduino.h"
#include "SPI.h"
#include "WiFiNINA.h"
#include "string.h"
#include <ArduinoJson.h>
#include <ArduinoMqttClient.h>

class ThingWorx {
	public:
		ThingWorx(char* server, int port, char* appKey, char* thingName, String* property_list,char* SSID, char* WifiPWD);
		void put(String property, float value);
		float getjson(String property);
    	void Wifi();
		void put_many(float value[], int size);
		String create_json(float value[], int size);

	private:
   		WiFiSSLClient _client;
		char* _server;
    	int _port;
		char* _appKey;
		char* _thingName;
		String* _property_list;
		char* _SSID;
		char* _WifiPWD;
};

class ThingWorx_MQTT {
	public:
		ThingWorx_MQTT(char* broker, int port, char* topic,char* SSID, char* WifiPWD);
		void mqtt_publish(String value);
		void broker_connect();
		void Wifi();
		
	private:
		char* _broker;
		int _port;
		char* _topic;
		char* _SSID;
		char* _WifiPWD;
};

#endif

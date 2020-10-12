/**********************************************************************************************************
 * Thingworx_MKRWifi1010.cpp - Library for up- and downloading values to/from a Thingworx "Thing" with an Arduino Wifi MKR1010
 * Created by Armin Fischer, Oct 2020
 * Version 3.0
 * School: HTBLuVA Wiener Neustadt
 * E-Mail: fia@htlwrn.ac.at
 * 
 * The REST API documentation was obtained from:
 * https://support.ptc.com/appserver/cs/view/solution.jsp?n=CS249622&art_lang=en&posno=1&q=Thingworx%20post%20request%20arduino&ProductFamily=ThingWorx%7CNRN%7CAxeda&source=search
 * 
 * GET Request example:
 * GET /Thingworx/Things/MyThing/Properties/MyProperty HTTP/1.1
 * Host: thingworx.myserver.com
 * Accept: application/json
 * Connection: close
 * appKey: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 * 
 * PUT Request example:
 * Connected to: xxxxxxxxxx:xxx
 * PUT (Host)/Thingworx/Things/(Thing)/(Property)?appKey=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 * Host: thingworx.myserver.com
 * Thing/Property: (Thing)/(Property)
 * appKey: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 * 
 * JSON-body: {"(Property)":20.50}
************************************************************************************************************/

#include "Thingworx_MKRWifi1010.h"

ThingWorx::ThingWorx(char* server, int port, char* appKey, char* thingName, String* property_list) {
  _server = server;
  _port = port;
  _appKey = appKey;
  _thingName = thingName;
  _property_list = property_list;
}

void ThingWorx::put(String property, float value) {
  String url = "/Thingworx/Things/";
  String body = "";
  url += _thingName;
  url += "/Properties/";
  url += property;
    body += "{\"";
    body += property;
    body += "\":";
    body += String(value);
    body += "}";
  
  if (_client.connect(_server, _port)) {
    Serial.println("Connected to: " + String(_server) + ":" + String(_port));
    //Send the HTTP PUT request:
    _client.print(String("PUT ") + url + " HTTP/1.1\r\n" +
    "Host: " + _server + "\r\n" +
    "Content-Type: application/json\r\n" +
    "Content-Length: " + body.length() + "\r\n" +
    "Connection: close\r\n" +
    "x-thingworx-session: false\r\n" +
    "appKey: " + _appKey + "\r\n\r\n" +
    body + "\r\n\r\n");

//    Serial.print(String("PUT ") + _server + url + "?appKey=" + _appKey + "\r\n" +
//    "Host: " + _server + "\r\n" +
//    "Thing/Property: " + _thingName + "/" + property + "\r\n" +
//    "appKey: " + _appKey + "\r\n\r\n" +
//    "JSON-body: " + body + "\r\n\r\n");

	Serial.print(String("PUT ") + url + " HTTP/1.1\r\n" +
    "Host: " + _server + "\r\n" +
    "Content-Type: application/json\r\n" +
    "Content-Length: " + body.length() + "\r\n" +
    "Connection: close\r\n" +
    "x-thingworx-session: false\r\n" +
    "appKey: " + _appKey + "\r\n\r\n" +
    body + "\r\n\r\n");

    unsigned long timeout = millis();
    while (_client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        _client.stop();
        return;
      }
    }
    Serial.println("***ANTWORT VOM CLIENT:***");
    while (_client.available()) {
      char c = _client.read();
      Serial.write(c);
    }
    Serial.println("*************************");
    _client.stop();
  }
  else {
    Serial.println("The connection could not be established");
    _client.stop();
  }
}

float ThingWorx::getjson(String property) {
  String url = "/Thingworx/Things/";
  url += _thingName;
  url += "/Properties/";
  url += property;
  if (_client.connect(_server, _port)) {
    Serial.println("Connected to: " + String(_server) + ":" + String(_port));

    //Send the HTTP GET request:
    _client.print(String("GET ") + url + " HTTP/1.1\r\n" +
    "Host: " + _server + "\r\n" +
    "Accept: application/json\r\n" +
    "Connection: close\r\n" +
    "appKey: " + _appKey + "\r\n\r\n");

     Serial.print(String("GET ") + _server + url + "?appKey=" + _appKey + "\r\n" +
    "Host: " + _server + "\r\n" +
    "Thing/Property: " + _thingName + "/" + property + "\r\n" +
    "appKey: " + _appKey + "\r\n\r\n" );

    unsigned long timeout = millis();
    while (_client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        _client.stop();
      }
    }
    String json = "";
    int i=0;
    char client_text[1000];
    float thingvalue;
    Serial.println("***ANTWORT VOM CLIENT:***");
    while (_client.available()) {
    	char c = _client.read();
	   	client_text[i]=c;
   		Serial.write(c);								//Auskommentieren um gesamte Clientkommunikation zu sehen
		i++;
		client_text[i]='\0';
    }
    Serial.println("*************************");
    //Ist Fehler aufgetreten?
    char* ptr = strstr(client_text,"HTTP/1.1");
  	int http_number = atoi(ptr + 9);
  	
    //Kein Fehler
    if(http_number==200)
    {
    	json=client_text; 									//Convert char [] to String
    	int colonPosition = json.lastIndexOf(':')+1;
    	String thingvalue_String = json.substring(colonPosition);
    	thingvalue= thingvalue_String.toFloat();
	}
	//Fehler - nAn wird ausgegeben	
	else
	{	unsigned long * p_flt = (unsigned long *)&thingvalue;
   		*p_flt = 0xFFFFFFFF;
	}
	Serial.println(String("Value: ")+ thingvalue);
	Serial.println();
	Serial.println();
    return thingvalue;
  }
  else {
    Serial.println("The connection could not be established");
    _client.stop();
  }
}

void ThingWorx::Wifi(char* ssid,char* password) {
  
delay(1000);  //Wait 1 sec for module initialization

  //Check if WiFi Shield is connected to Arduino
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    //Infinite loop if shield is not detected
    while (true);
  }

  //Attempt a WiFi connection to desired access point at ssid, password
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    WiFi.begin(ssid, password);
    delay(10000); //Wait 10 secs for establishing connection
  }
  //Print WiFi status
 //Print SSID name
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  //Print ipv4 assigned to WiFi101 module
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  //Print signal strength for WiFi101 module
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

//*********************************************************************
//Version 2.0 programming starts here
//*********************************************************************

void ThingWorx::put_many(float value[], int size) {
  String url = "/Thingworx/Things/";
  String body = "";
  url += _thingName;
  url += "/Properties/*";
    body += create_json(value,size);
  
  if (_client.connect(_server, _port)) {
    Serial.println("Connected to: " + String(_server) + ":" + String(_port));
    //Send the HTTP PUT request:
    _client.print(String("PUT ") + url + " HTTP/1.1\r\n" +
    "Host: " + _server + "\r\n" +
    "Content-Type: application/json\r\n" +
    "Content-Length: " + body.length() + "\r\n" +
    "Connection: close\r\n" +
    "x-thingworx-session: false\r\n" +
    "appKey: " + _appKey + "\r\n\r\n" +
    body + "\r\n\r\n");

//    Serial.print(String("PUT ") + _server + url + "?appKey=" + _appKey + "\r\n" +
//    "Host: " + _server + "\r\n" +
//    "Thing/Property: " + _thingName + "/" + property + "\r\n" +
//    "appKey: " + _appKey + "\r\n\r\n" +
//    "JSON-body: " + body + "\r\n\r\n");

	Serial.print(String("PUT ") + url + " HTTP/1.1\r\n" +
    "Host: " + _server + "\r\n" +
    "Content-Type: application/json\r\n" +
    "Content-Length: " + body.length() + "\r\n" +
    "Connection: close\r\n" +
    "x-thingworx-session: false\r\n" +
    "appKey: " + _appKey + "\r\n\r\n" +
    body + "\r\n\r\n");

    unsigned long timeout = millis();
    while (_client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        _client.stop();
        return;
      }
    }
    Serial.println("***ANTWORT VOM CLIENT:***");
    while (_client.available()) {
      char c = _client.read();
      Serial.write(c);
    }
    Serial.println("*************************");
    _client.stop();
  }
  else {
    Serial.println("The connection could not be established");
    _client.stop();
  }
}

String ThingWorx::create_json(float value_1[], int size) {
	//StaticJsonDocument<200> doc;	
	DynamicJsonDocument doc(2048);
	String json;
	for(int i=0;i<size;i++){
		doc[_property_list[i]].set(value_1[i]);
	}
	serializeJson(doc, json);
	return json;
}



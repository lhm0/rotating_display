// =========================================================================================================================================
//                                                 Rotating Display RD40
//                                                    © Ludwin Monz
// License: Creative Commons Attribution - Non-Commercial - Share Alike (CC BY-NC-SA)
// you may use, adapt, share. If you share, "share alike".
// you may not use this software for commercial purposes 
// =========================================================================================================================================

#include "my_ESP.h"
#include "RD_40.h"
#include "FlashFS.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FlashFS.h>
#include <Wire.h>

#define _SDA 2
#define _SCL 0
#define TIME_ZONE "CET-1CEST,M3.5.0/02,M10.5.0/03" // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv    
                                                   // this is only the default (my home ;-)       
#define NTP "pool.ntp.org"

// =========================================================================================================================================
//                                                      Constructor
// =========================================================================================================================================

my_ESP::my_ESP() {}

// =========================================================================================================================================
//                                                      begin Method
// =========================================================================================================================================

void my_ESP::begin() {

  // initiate Wifi
  _iniWifi();

  // set time
  setMyTime();

  // Setup I2C
  Wire.begin(_SDA, _SCL); // SDA=GPIO2 /  SCL=GPIO0 ==> registers the controller as I2C Master
  Wire.setClock(200000L);
}

// =========================================================================================================================================
//                                                      Wifi Methods
// =========================================================================================================================================

void my_ESP::_iniWifi() {
  /*
    read SSID and password, if available
  */
  _ssid_f.begin();
  _ssid = _ssid_f.read_f();
  _password = _password_f.read_f();
  Serial.println(_ssid);
  Serial.println(_password);

  /*
    try to connect to WiFi, if ssid and password are valid
  */
  if ((_ssid!="")|(_password!="")) { 
    WiFi.begin(_ssid, _password);

    Serial.print("connecting to WiFi ...");

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      delay(1000);
      Serial.print(".");
      attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
      ssid = _ssid;
      ipAddress = WiFi.localIP().toString();
      Serial.println("WiFi connected");
  } else {
      Serial.println("Wifi connection failed...");
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    ssid = "RD40";
    WiFi.softAP(ssid,"",1,false,5);

    ipAddress = WiFi.softAPIP().toString();

    Serial.println("Access Point started.");
    Serial.print("IP-Adresse: ");
    Serial.println(ipAddress);  
  }

}

// =========================================================================================================================================
//                                                      Time Methods
//                                  https://werner.rothschopf.net/202011_arduino_esp8266_ntp_en.htm
// =========================================================================================================================================

void my_ESP::setMyTime() {
  _timeZone = _timeZone_f.read_f();
  if (_timeZone == "") _timeZone = "CET-1CEST,M3.5.0/02,M10.5.0/03";
  char _timeZone_c[50];
  strcpy(_timeZone_c, _timeZone.c_str());
  configTime(_timeZone_c, NTP);
}

void my_ESP::getMyTime() {
  time(&_now);                 // read the current time
  localtime_r(&_now, &tm1);    // update the structure tm1 with the current time

  Hour = tm1.tm_hour;
  Min = tm1.tm_min;
  Sec = tm1.tm_sec;
  Mday = tm1.tm_mday;
  Mon = tm1.tm_mon + 1;
  Year = tm1.tm_year + 1900;
  Wday = tm1.tm_wday;
}

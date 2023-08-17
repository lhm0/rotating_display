// =========================================================================================================================================
//                                                 Rotating Display RD40
//                                                    © Ludwin Monz
// License: Creative Commons Attribution - Non-Commercial - Share Alike (CC BY-NC-SA)
// you may use, adapt, share. If you share, "share alike".
// you may not use this software for commercial purposes 
// =========================================================================================================================================


// ******************************************************************************************************************************************
//
//          This class manages the ressources of the mikrocontroller:
//              * Wifi
//              * I2C Interface
//              * clock
//              * data and file storage
//
// ******************************************************************************************************************************************

#ifndef my_ESP_H
#define my_ESP_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "FlashFS.h"

class my_ESP {
  private:
  
    void _iniWifi();                       // either connects to known Wifi or starts AP

    time_t _now;                           // this is the epoch

    FlashFS _ssid_f{"/variables/ssid.txt"};
    FlashFS _password_f{"/variables/password.txt"};
    FlashFS _timeZone_f{"/variables/timeZone.txt"};

    String _ssid;                          // ssid read from ssid.txt
    String _password;                      // password read from password.txt
    String _timeZone;                      // POSIX for selected time zone

  public:
    my_ESP();                              // Constructor

    void begin();                          // initiate Wifi
                                           // configure I2C
                                           // set time

    AsyncWebServer server{80};             // AsyncWebServer: Server object is public

    String ipAddress;
    String ssid;

    tm tm1;                                // the structure tm holds time information 
    int Sec;   // Sekunden (0-59)
    int Min;   // Minuten (0-59)
    int Hour;  // Stunden (0-23)
    int Mday;  // Tag des Monats (1-31)
    int Mon;   // Monat (0-11)
    int Year;  // Jahr - 1900
    int Wday;  // Tag der Woche (0-6, Sonntag = 0)

    void setMyTime();
    void getMyTime();

};

#endif

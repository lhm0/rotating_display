// =========================================================================================================================================
//                                                 Rotating Display RD40
//                                                    © Ludwin Monz
// License: Creative Commons Attribution - Non-Commercial - Share Alike (CC BY-NC-SA)
// you may use, adapt, share. If you share, "share alike".
// you may not use this software for commercial purposes 
// =========================================================================================================================================


#include <Arduino.h>

#include "my_ESP.h"
#include "RD_40.h"
#include "RD_40trafo.h"
#include "my_BMP.h"
#include "my_BMPtemplates.h"
#include "FlashFS.h"
#include "webInterface.h"


// =========================================================================================================================================
// 
//                                                        Rotating Display
//
// The program uses OOP in order to structure data and functions. These are the classes:
//
// 1. my_ESP
//    description:    the class manages the resources of the ESP microcontroller. 
//    key methods:    begin()                                         configures Wifi, i2c interface, sets time 
//                    getMyTime()                                     occupies the time and date attributes (see below) 
//                    myPassword()                                    returns Wifi Password which is being used
//                    mySSID()                                        returns Wifi SSID
//    key attributes: int Sec, int Min, int Hour, ...
//
// 2. RD_40
//    description:    the class provides the central rotating display object. It manages the data of the displayed image (one bit per LED), 
//                    etc. 
//                    The class has methods for displaying bitmaps and for transferring data to the display controller. 
//    key methods:    begin()                                         initialize display mode and data transfer to the display controller
//                    upload()                                        uploads display data to the display controller
//                    displayBMP(unsigned char bmp[][14])             displays bitmap
//    key attributes: clockMode, brightness
//
// 3. my_BMP
//    description:    this class generates bitmaps for the display. It provides methods for generating clock and weather screens, 
//                    as well as printing text on bitmaps. The bitmap shall be handed over to the RD40 object. 
//    key methods:    generateBMP(int mode, tm* tm, String* ssid)     generates bitmap according to mode, time, ssid
//    key attributes: bitmap[][]                                      this is a 110x110 pixel bitmap
//
// 4. webInterface
//    description:    the class manages the web user interface. It uses the AsyncWebServer library and runs in the background without any need for
//                    attention. The class provides certain attributes, which are set by the web interface:
//    key methods:    begin(String* ssid_)                            starts AsyncWebServer. ssid is displayed (not changed) in web interface
//    key attributes: int clockMode;                                  mode 0 = IP-address, mode 1 = analog clock, mode 2 = digital clock, mode 3 = logo clock, mode 4 = weather clock
//                    int brightness;                                 brightness value in %
//                    char apiKey[50]="";                             apiKey for openweathermap.org
//                    char location[50]="";                           location for openweathermap.org
//                    char country[20]="";                            country for openweathermap.org
//
//
// =========================================================================================================================================


my_ESP myESP;                                                // create instance of my_ESP
webInterface wi40;                                           // create instance of webInterface
RD_40 RD40;                                                  // create instance of RD_40
my_BMP myBMP;                                                // create instance of my_BMP

long lastWeather;                                            // remember time of last weather updte
int lastUpdate=0;                                            // remember the second of last display update
int prevClockMode=4;                                         // remember previous clockMode

// =========================================================================================================================

void setup(){

  // Serial port for debugging purposes
  Serial.begin(115200);


  myESP.begin();                            // initiates Wifi, I2C, time, and SPIFFS file management
  String myssid = myESP.ssid;               // withdraw SSID
  String myIpAddress = myESP.ipAddress;
  wi40.begin(myssid);                       // start web server

  myESP.getMyTime();
  Serial.printf("Zeit: %d:%d:%d\n", myESP.Hour, myESP.Min, myESP.Sec);
}

void loop() {
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 
//


    if ( (((millis()-lastWeather)>60000)&&(wi40.clockMode==4)) || wi40.updateWeather) {     // update weather every 60 seconds OR if update requested with update_weather
      lastWeather=millis();
      Serial.println("request weather data.....");
      myBMP.getWeather(wi40.apiKey, wi40.location);
      wi40.updateWI(myBMP.w_icon, myBMP.w_temp, myBMP.w_humi);                               //send data via WebSocket to webpage

      wi40.updateWeather=false;
    }

    myESP.getMyTime();

    if ((lastUpdate!=myESP.Sec)&&(RD40.setComplete)) {  

      lastUpdate=myESP.Sec;
      Serial.printf("time: %d:%d:%d\n",myESP.Hour,myESP.Min,myESP.Sec);
      
      if (wi40.clockMode!=prevClockMode) {
        prevClockMode=wi40.clockMode;
        RD40.sendAll=true;
      }

      int myMode = wi40.clockMode;                                      // wi40 holds clockMode setting of user interface
      tm* mytm = &myESP.tm1;                                            // myESP holds time
      String myssid = myESP.ssid;                                       // withdraw SSID
      String myIpAddress = myESP.ipAddress;     

      myBMP.generateBMP(myMode, mytm, myssid.c_str(), myIpAddress.c_str());  // generate bitmap

      unsigned char (*mybitmap)[14] = myBMP.bitmap;                     // pointer at bitmap array
      RD40.displayBMP(mybitmap);                                        // update display data according to bitmap

      int mybrightness = wi40.brightness;                               // wi40 holds the brightness setting of user interface
      RD40.brightness = mybrightness;                                   // update brightness. 

      if (myMode == 6) RD40.animation_mode = true;
      if ((myMode != 6)&&(RD40.animation_mode = true)) {
        RD40.animation_mode = false;
        RD40.sendAll = true;
      }

      RD40.upload(mybrightness);                                        // 

    }
}

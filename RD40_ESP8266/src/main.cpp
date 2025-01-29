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
// =========================================================================================================================================

my_ESP myESP;                                                // create instance of my_ESP
webInterface wi40;                                           // create instance of webInterface
RD_40 RD40;                                                  // create instance of RD_40
my_BMP myBMP;                                                // create instance of my_BMP

long lastWeatherUpdateTime = 0;                              // remember time of last weather update
int lastDisplayUpdateSecond = 0;                             // remember the second of last display update
int previousClockMode = 4;                                   // remember previous clockMode

// =========================================================================================================================

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  myESP.begin();                            // initiates Wifi, I2C, time, and SPIFFS file management
  String ssid = myESP.ssid;                 // withdraw SSID
  wi40.begin(ssid);                         // start web server

  myESP.getMyTime();
  Serial.printf("Current Time: %d:%d:%d\n", myESP.Hour, myESP.Min, myESP.Sec);
}

void loop() {
  // Update weather data every 60 seconds or if an update is requested
  if (((millis() - lastWeatherUpdateTime) > 60000 && wi40.clockMode == 4) || wi40.updateWeather) {
    lastWeatherUpdateTime = millis();
    Serial.println("Requesting weather data...");
    myBMP.getWeather(wi40.apiKey, wi40.location);
    wi40.updateWI(myBMP.w_icon, myBMP.w_temp, myBMP.w_humi);  // Send data via WebSocket to webpage
    wi40.updateWeather = false;
  }

  myESP.getMyTime();

  // Update display every second if the display is ready
  if ((lastDisplayUpdateSecond != myESP.Sec) && RD40.setComplete) {
    lastDisplayUpdateSecond = myESP.Sec;
    Serial.printf("Current Time: %d:%d:%d\n", myESP.Hour, myESP.Min, myESP.Sec);

    // Check if clock mode has changed
    if (wi40.clockMode != previousClockMode) {
      previousClockMode = wi40.clockMode;
      RD40.sendAll = true;
    }

    int currentClockMode = wi40.clockMode;                   // wi40 holds clockMode setting of user interface
    tm* currentTime = &myESP.tm1;                            // myESP holds time
    String ssid = myESP.ssid;                                // withdraw SSID
    String ipAddress = myESP.ipAddress;

    myBMP.generateBMP(currentClockMode, currentTime, ssid.c_str(), ipAddress.c_str());  // generate bitmap

    unsigned char (*bitmap)[14] = myBMP.bitmap;              // pointer at bitmap array
    RD40.displayBMP(bitmap);                                 // update display data according to bitmap

    int currentBrightness = wi40.brightness;                 // wi40 holds the brightness setting of user interface
    RD40.brightness = currentBrightness;                     // update brightness

    // Handle animation mode
    if (currentClockMode == 6) {
      RD40.animation_mode = true;
    } else if (RD40.animation_mode) {
      RD40.animation_mode = false;
      RD40.sendAll = true;
    }

    RD40.upload(currentBrightness);                          // Upload display data
  }
}
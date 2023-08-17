// =========================================================================================================================================
//                                                 Rotating Display RD40
//                                                    © Ludwin Monz
// License: Creative Commons Attribution - Non-Commercial - Share Alike (CC BY-NC-SA)
// you may use, adapt, share. If you share, "share alike".
// you may not use this software for commercial purposes 
// =========================================================================================================================================



// ******************************************************************************************************************************************
//
//          This class provides the user interface for controlling the rotating display
//
//          The initialization method "webInterface::begin(my_ESP* myESP) uses the AsyncWebServer object "server" from myESP.
//          The web pages are stored in the SPIFFS ny means of LittleFS. 
//
// ******************************************************************************************************************************************


#ifndef webInterface_H
#define webInterface_H

#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "my_ESP.h"
#include "FlashFS.h"

class webInterface {
  private:
    AsyncWebServer _server{80};                 // server object
    AsyncWebSocket _ws{"/ws"};                  // ws object

    FlashFS _apiKey_f{"/variables/apiKey.txt"};
    FlashFS _location_f{"/variables/location.txt"};
    FlashFS _country_f{"/variables/country.txt"};
    FlashFS _logoPath_f{"/variables/logoPath.txt"};
    FlashFS _clockFacePath_f{"/variables/clockFacePath.txt"};
    FlashFS _imagePath_f{"/variables/imagePath.txt"};
    FlashFS _ssid_f{"/variables/ssid.txt"};
    FlashFS _password_f{"/variables/password.txt"};
    FlashFS _timeZone_f{"/variables/timeZone.txt"};
    static String _currentPath;

    int _w_icon=0;
    String _w_temp="99";
    String _w_humi="99";

    String _brightness_s;
    String _ssid; 

    static webInterface* _instance;

    void _startServer();
    static void _handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void _handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
    void _onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);


  public:
    webInterface();                                             // constructor

    void begin(String ssid_);                                  // starts the web server
    int clockMode=0;
    int brightness=50;
    
    bool updateWeather=true;                                    // if true, main loop updates weather data
    String apiKey="";                                           // apiKey for openweathermap.org
    String location="";                                         // location for openweathermap.org
    String country="";                                          // country for openweathermap.org
    String logoPath="";                                         // path of logo file
    String clockFacePath="";                                    // path of clock face
    String imagePath="";                                        // path of image
    void updateWI(int w_icon, String w_temp, String w_humi);    // update web interface with weather data

};

#endif    

// =========================================================================================================================================
//                                                 Rotating Display RD40
//                                                    © Ludwin Monz
// License: Creative Commons Attribution - Non-Commercial - Share Alike (CC BY-NC-SA)
// you may use, adapt, share. If you share, "share alike".
// you may not use this software for commercial purposes 
// =========================================================================================================================================


// ******************************************************************************************************************************************
//
//          this class generates a bitmap for the display. It provides methods for generating clock and weather screens,
//          as well as printing text on the bitmap. The bitmap shall be handed over to the RD40 object. 
//          
// ******************************************************************************************************************************************


#ifndef my_BMP_H
#define my_BMP_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

class my_BMP {
  private:
    void _generateIP();                                 // generates bitmap, which shows the IP address
    void _generateAnalog();                             // generates bitmap with analog clock
    void _generateDigital();                            // generates bitmap with digital clock
    void _generateLogoClock();                          // generates bitmap with logo clock
    void _generateWeather();                            // generates bitmap with weather information
    void _generateImage();                              // generates bitmap with image
    
    bool _loadLogo();
    bool _loadWatchFace();
    void _clr_bmp();

    void _setPixel(int x, int y);                                   // set pixel in bitmap[][]
    void _clrPixel(int x, int y);                                   // clear pixel in bitmap[][]
    void _clrBox(int x1, int y1, int x2, int y2);                   // clear Box in bitmap[][]
    void _drawLine(int x1, int y1, int x2, int y2);                 // draw line in bitmap[][]
    void _drawCircle(int radius);               // draw circle in bitmap[][]
    void _drawRadius(int n, int rStart, int rEnd);                  
    int _polarToX(int n, int r) ;                                   // transform polar coordinates to carthesian coordinate x
    int _polarToY(int n, int r) ;                                   // transform polar coordinates to carthesian coordinate y
                                                                    // n corresponds to the angle, r to the radius of the polar coordinate system-
                                                                    // range of n = 0,...,239
                                                                    // range of r = 0,...,39

    void _print_16x24(int p_mode, char s[], int xpos, int ypos);
    void _print_12x18(int p_mode, char s[], int xpos, int ypos);
    void _print_10x15(int p_mode, char s[], int xpos, int ypos);
    void _print_icon_30x20(int p_mode, int i_num, int xpos, int ypos);

    int _iconNumber(String iconText);

    int _myYear;
    int _myMon;
    int _myMday;
    int _myHour;
    int _myMin;
    int _mySec;
    int _myWday;

    char _ssid[50]; 
    char _ipAddress[50];

  public:
    unsigned char bitmap[110][14];                                // bitmap with x=110 and y = 14 bytes * 8 bits = 112
    
    int w_icon;                                                  // weather data
    String w_temp;
    String w_humi;
    
    my_BMP();                                                     // Constructor
    void generateBMP(int mode, tm* tm1, const char* ssid, const char* ipAddress);        // generates bitmap according to mode, time, ssid

    void getWeather(String apiKey, String location);
};

#endif

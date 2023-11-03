// =========================================================================================================================================
//                                                 Rotating Display RD40
//                                                    © Ludwin Monz
// License: Creative Commons Attribution - Non-Commercial - Share Alike (CC BY-NC-SA)
// you may use, adapt, share. If you share, "share alike".
// you may not use this software for commercial purposes 
// =========================================================================================================================================


#include "my_BMP.h"
#include "FlashFS.h"
#include "my_BMPtemplates.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

// =========================================================================================================================================
//                                                      Constructor
// =========================================================================================================================================

my_BMP::my_BMP() {
}


// =========================================================================================================================================
//                                                      generateBMP method
// =========================================================================================================================================

void my_BMP::generateBMP(int mode, tm* tm1, const char* ssid, const char* ipAddress) {      // generates bitmap according to mode
  _myYear = tm1->tm_year;                                       // save tm to private attributes
  _myMon = tm1->tm_mon;
  _myMday = tm1->tm_mday;
  _myHour = tm1->tm_hour;
  _myMin = tm1->tm_min;
  _mySec = tm1->tm_sec;
  _myWday = tm1->tm_wday;

   strcpy(_ssid, ssid);                                        // save ssid to private attribute
   strcpy(_ipAddress, ipAddress);                              // save ipAddress to private attribute

   if (mode==0) _generateIP();                                 // generates bitmap, which shows the IP address
   if (mode==1) _generateAnalog();                             // generates bitmap with analog clock
   if (mode==2) _generateDigital();                            // generates bitmap with digital clock
   if (mode==3) _generateLogoClock();                          // generates bitmap with logo clock
   if (mode==4) _generateWeather();                            // generates bitmap with weather information
   if (mode==5) _generateImage();                            // generates bitmap with image
}

// =========================================================================================================================================
//                                                      _generateIP method
// =========================================================================================================================================

void my_BMP::_generateIP() {
    char text1[100];
    char ip_addr[] = "0.0.0.0";
    char parsed_ip[4][4];
    char ip_part1[20];
    char ip_part2[20];
    char* token;
//    strcpy(ip_addr,_ipAddress.toString().c_str());

    token = strtok(_ipAddress, ".");
    int i = 0;
    while (token != NULL) {
      strcpy(parsed_ip[i], token);
      token = strtok(NULL, ".");
      i++;
    }
    strcpy(ip_part1, parsed_ip[0]);
    strcat(ip_part1, ".");
    strcat(ip_part1, parsed_ip[1]);

    strcpy(ip_part2, parsed_ip[2]);
    strcat(ip_part2, ".");
    strcat(ip_part2, parsed_ip[3]);

    strcpy(text1, ".");
    strcat(ip_part1, text1);    //append "."

    _clr_bmp();     // clear bitmap

    _print_10x15(1, ip_part1, 55, 25);
    _print_10x15(1, ip_part2, 55, 9);

    strcpy(text1,_ssid);
    _print_10x15(1, text1, 55, 72);
    
    text1[0]=0x7E;              // Wifi symbol
    text1[1]=0x7F;
    text1[2]=0x00;
    _print_10x15(1, text1, 55, 88);
}

// =========================================================================================================================================
//                                                      _generateAnalog() method
// =========================================================================================================================================

void my_BMP::_generateAnalog() {
  if (!_loadWatchFace()) {
    _clr_bmp();
    _drawCircle(38);
    _drawCircle(39);
    for (int n=0; n<240; n=n+20) _drawRadius(n, 32, 39);
    for (int n=0; n<240; n=n+4) _drawRadius(n, 35, 39);
    char text[10]="12";
    _print_10x15(1, text, 55, 95);
    strcpy(text,"6");
    _print_10x15(1, text, 55, 10);
    strcpy(text,"3");
    _print_10x15(2, text, 100, 50);
    strcpy(text,"9");
    _print_10x15(1, text, 15, 50);
  }

  // hour hand
    float hour12;
    if (_myHour>12) hour12=_myHour-12;
    else hour12=_myHour;
    int n = (int)((hour12*60+_myMin)/3);
    _drawRadius(n ,0 , 20);

  // minute hand
    n = (int)((_myMin*60+_mySec)/15);
    _drawRadius(n ,0 , 35);

  // second hand
    n = _mySec*4;
    _drawRadius(n ,0 , 40);
}

bool my_BMP::_loadWatchFace() {
//  retrieve logoPath
    FlashFS wfPath_f{"/variables/clockFacePath.txt"};
    String wfPath = wfPath_f.read_f();
    
// load rd40 image data
    FlashFS rd40_f(wfPath);
    return rd40_f.read_f(bitmap, 110);
}


// =========================================================================================================================================
//                                                      _generateDigital() method
// =========================================================================================================================================
void my_BMP::_generateDigital() {
        char text4[64];
        _clr_bmp();                                           // clear bitmap
        sprintf(text4,":");
        _print_12x18(1,text4,40,72);
        _print_12x18(1,text4,70,72);
        sprintf(text4,"%02d",_myHour);
        _print_12x18(1,text4,25,72);
        sprintf(text4,"%02d",_myMin);
        _print_12x18(1,text4,55,72);
        sprintf(text4,"%02d",_mySec);
        _print_12x18(1,text4,85,72);
        sprintf(text4,"%02d.%02d.%04d",_myMday, _myMon+1, _myYear+1900);
        _print_10x15(1,text4,55,24);
        if (_myWday==0) strcpy(text4,"SUN");
        if (_myWday==1) strcpy(text4,"MON");
        if (_myWday==2) strcpy(text4,"TUE");
        if (_myWday==3) strcpy(text4,"WED");
        if (_myWday==4) strcpy(text4,"THU");
        if (_myWday==5) strcpy(text4,"FRI");
        if (_myWday==6) strcpy(text4,"SAT");
        _print_10x15(1,text4,55,7);
}

// =========================================================================================================================================
//                                                      _generateLogoClock() method
// =========================================================================================================================================

void my_BMP::_generateLogoClock() {
    _loadLogo();                                         // copy logo bitmap to "bitmap"                                       
    _clrBox(12, 72, 98, 102);

    char text4[64];
    sprintf(text4, ":");
    _print_16x24(1,text4,55,75);

    sprintf(text4,"%02d", _myHour);           
    _print_16x24(0,text4,15,75);

    sprintf(text4,"%02d", _myMin);           
    _print_16x24(0,text4,59,75);

    _clrBox(67, 45, 103, 66);

    sprintf(text4,":%02d", _mySec);          
    _print_10x15(0,text4,70,48);
}

bool my_BMP::_loadLogo() {
//  retrieve logoPath
    FlashFS logoPath_f{"/variables/logoPath.txt"};
    String logoPath = logoPath_f.read_f();
    
// load rd40 image data
    FlashFS rd40_f(logoPath);
    return rd40_f.read_f(bitmap, 110);
}

// =========================================================================================================================================
//                                                      _generateWeather() method
// =========================================================================================================================================

void my_BMP::_generateWeather() {
        _clr_bmp();

        char text4[64];
        sprintf(text4, ":");
        _print_16x24(1,text4,55,75);
        sprintf(text4,"%02d", _myHour);          
        _print_16x24(0,text4,15,75);
        sprintf(text4,"%02d", _myMin);          
        _print_16x24(0,text4,59,75);
        sprintf(text4,":%02d", _mySec);
        _print_10x15(0,text4,70,48);

        char text5[20];
        w_temp.toCharArray(text5, 20);
        sprintf(text4, "%s'C", text5);        
        _print_10x15(1,text4,55,24);
        w_humi.toCharArray(text5, 20);
        sprintf(text4, "%s%%", text5);        
        _print_10x15(1,text4,55,8);
        _print_icon_30x20(1,w_icon, 20, 47);
        
}

// =========================================================================================================================================
//                                                      _generateImage() method
// =========================================================================================================================================

void my_BMP::_generateImage() {
//  retrieve imagePath
    FlashFS imagePath_f{"/variables/imagePath.txt"};
    String imagePath = imagePath_f.read_f();
    
// load rd40 image data
    FlashFS rd40_f(imagePath);
    rd40_f.read_f(bitmap, 110);
}

// =========================================================================================================================================
//                                                      clr_bmp method
// =========================================================================================================================================

void my_BMP::_clr_bmp() {
  int xa, ya;
  for (xa=0; xa<110; xa++) {
    for (ya=0; ya<14; ya++) bitmap[xa][ya]=0x00;
  }
}

// =========================================================================================================================================
//                        methods for clearing and setting bitmap pixels, clearing bitmap areas and drawing lines
// =========================================================================================================================================

void my_BMP::_setPixel(int x, int y) {
  bitmap[x][(int)(y/8)] |= (1 << (y%8));
}

void my_BMP::_clrPixel(int x, int y) {
  bitmap[x][(int)(y/8)] &= ~(1 << (y%8));
}

void my_BMP::_clrBox(int x1, int y1, int x2, int y2) {
  if (x1 > x2) {
    int aux = x1;
    x1 = x2;
    x2 = aux;
  }
  if (y1 > y2) {
    int aux = y2;
    y1 = y2;
    y2 = aux;
  }
  for (int x=x1; x<x2; x++) {
    for (int y=y1; y<y2; y++) _clrPixel(x, y);
  }
}

void my_BMP::_drawLine(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        _setPixel(x1, y1);

        if (x1 == x2 && y1 == y2)
            break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void my_BMP::_drawCircle(int radius) {
  int r = radius;
  for (int n=0; n<240; n++) _setPixel(_polarToX(n, r), _polarToY(n, r));
}

void my_BMP::_drawRadius(int n, int rStart, int rEnd) {
  int r=0;
  for (r=rStart; r<=rEnd; r++) {
    _setPixel(_polarToX(n, r), _polarToY(n, r));
  }
}

int my_BMP::_polarToX(int n, int r) {
  float theta = 0.026179939*(n+0.5);                // 2*pi/240 = 0.026179939
  return (int)roundf(54.5+(r+15.5)*sin(theta));
}

int my_BMP::_polarToY(int n, int r) {
  float theta = 0.026179939*(n+0.5);                // 2*pi/240 = 0.026179939
  return (int)roundf(54.5+(r+15.5)*cos(theta));
}


// =========================================================================================================================================
//                                  methods for printing characters, numbers, and weather icons on bitmap
// =========================================================================================================================================

void my_BMP::_print_16x24(int p_mode, char s[], int xpos, int ypos) {             // p_mode = print mode
  int l,j,x;                                                                      // p_mode = 0 => right
  unsigned long chr;                                                              // p_mode = 1 => mid
  l= strlen(s);                                                                   // p_mode = 2 => left

  if (p_mode==1) {
    xpos=xpos-9*l;
  }
  if (p_mode==2) {
    xpos=xpos-18*l;
  }

  for (j=0; j<l; j++) {
    for (x=0; x<16; x++) {
        chr=(unsigned long)pgm_read_dword(&chr_16x24[(int)s[j]-0x30][3*x]);
        chr&=0xFFFFFF;
        chr<<(ypos%8);
        memcpy(&bitmap[xpos+x+18*j][(int)(ypos/8)],&chr,3);
    }
  }
}

void my_BMP::_print_12x18(int p_mode, char s[], int xpos, int ypos) {                      // p_mode = print mode
  int l,j,x;                                                                      // p_mode = 0 => right
  unsigned long chr;                                                              // p_mode = 1 => mid
  l= strlen(s);                                                                   // p_mode = 2 => left

  if (p_mode==1) {
    xpos=xpos-6*l;
  }
  if (p_mode==2) {
    xpos=xpos-12*l;
  }

  for (j=0; j<l; j++) {
    for (x=0; x<12; x++) {
        chr=(unsigned long)pgm_read_dword(&chr_12x18[(int)s[j]-0x20][3*x]);
        chr&=0xFFFFFF;
        chr<<(ypos%8);
        memcpy(&bitmap[xpos+x+12*j][(int)(ypos/8)],&chr,3);
    }
  }
}

void my_BMP::_print_10x15(int p_mode, char s[], int xpos, int ypos) {                      // p_mode = print mode
  int l,j,x;                                                                      // p_mode = 0 => right
  unsigned long chr;                                                              // p_mode = 1 => mid
  l= strlen(s);                                                                   // p_mode = 2 => left

  if (p_mode==1) {
    xpos=xpos-5*l;
  }
  if (p_mode==2) {
    xpos=xpos-10*l;
  }

  for (j=0; j<l; j++) {
    for (x=0; x<10; x++) {
        chr=(unsigned long)pgm_read_dword(&chr_10x15[(int)s[j]-0x20][2*x]);
        chr&=0xFFFF;
        chr<<(ypos%8);
        memcpy(&bitmap[xpos+x+10*j][(int)(ypos/8)],&chr,2);
    }
  }
}

void my_BMP::_print_icon_30x20(int p_mode, int i_num, int xpos, int ypos) {
  int k,x;
  unsigned char chr;
  unsigned long long chr_l=0;

  if (p_mode==1) {
    xpos=xpos-15;
  }
  if (p_mode==2) {
    xpos=xpos-30;
  }

    for (x=0; x<30; x++) {
      for (k=0; k<3; k++) {
        chr_l<<=8;
        chr=pgm_read_byte(&icons_30x20[i_num][x][2-k]);
        memcpy(&chr_l,&chr,1);
      }
      chr_l<<(ypos%8);  
      memcpy(&bitmap[xpos+x][(int)(ypos/8)],&chr_l,3);
    }
}

// =========================================================================================================================================
//                                  method for dowloading weather data from openweathermap.org
//                      source: https://create.arduino.cc/projecthub/officine/getting-weather-data-655720
// =========================================================================================================================================

void my_BMP::getWeather(String apiKey, String location) {

    int status = WiFi.status();
    if (status==WL_CONNECTED) Serial.println("connected to Wifi...");
    else Serial.println("Wifi connection lost...");

    WiFiClient client;
    char server1[] = "api.openweathermap.org";     
    Serial.println("\nStarting connection to server..."); 
    if ((status==WL_CONNECTED)&&(client.connect(server1, 80))) { 
      Serial.println("connected to server"); 
      // Make a HTTP request: 
      client.print("GET /data/2.5/weather?"); 
      client.print("q="+location); 
      client.print("&appid="+apiKey); 
      client.println("&units=metric"); 
      client.println("Host: api.openweathermap.org"); 
      client.println("Connection: close"); 
      client.println(); 
    } else { 
      Serial.println("unable to connect"); 
      return;
    } 

    DynamicJsonDocument jsonDoc(2000);
  
    String line; 

    if (client.connected()) { 
      line="";
      int n = 0;
      while (line=="") {
        line = client.readStringUntil('\n'); 
        n++;
        if (n>20) return;
      }

      char line_c[line.length()];
      std::strcpy(line_c, line.c_str());
      size_t length = line.length();
      _hexDump(line_c, length);

      Serial.println("parsing values"); 

      if(line != "")  //Only do an update, if we got valid data
      {
          jsonDoc.clear();                                                //Normally not needed, but sometimes new data will not stored
          DeserializationError error = deserializeJson(jsonDoc, line);    //Deserialize string to AJSON-doc
          if (error)
          {
              Serial.print(F("deserializeJson() failed: "));
              Serial.println(error.c_str());
          }
  
          String text1;
          text1 = jsonDoc["weather"][0]["icon"].as<String>();
          w_icon = _iconNumber(text1);        
          w_temp = jsonDoc["main"]["temp"].as<String>();
          w_humi = jsonDoc["main"]["humidity"].as<String>();
  
          float f_temp = round(atof(w_temp.c_str()) * 10) / 10.0;         // round to 1 digit
          char w_temp_c[20];
          sprintf(w_temp_c,"%0.1f",f_temp);
          w_temp = String(w_temp_c);                                      // convert to String

          Serial.print("icon: ");
          Serial.println(w_icon);
          Serial.print("temp: "); 
          Serial.print(w_temp);
          Serial.println(" C");
          Serial.print("humidity: "); 
          Serial.print(w_humi);
          Serial.println(" %"); 
       }

      client.stop();
    }
}


int my_BMP::_iconNumber(String iconText) {
  int iconNr;
  iconNr=0;
  if (strcmp("01d",iconText.c_str())==0) iconNr=0;
  if (strcmp("01n",iconText.c_str())==0) iconNr=0;
  if (strcmp("02d",iconText.c_str())==0) iconNr=1;
  if (strcmp("02n",iconText.c_str())==0) iconNr=1;
  if (strcmp("03d",iconText.c_str())==0) iconNr=2;
  if (strcmp("03n",iconText.c_str())==0) iconNr=2;
  if (strcmp("04d",iconText.c_str())==0) iconNr=3;
  if (strcmp("04n",iconText.c_str())==0) iconNr=3;
  if (strcmp("09d",iconText.c_str())==0) iconNr=4;
  if (strcmp("09n",iconText.c_str())==0) iconNr=4;
  if (strcmp("10d",iconText.c_str())==0) iconNr=5;
  if (strcmp("10n",iconText.c_str())==0) iconNr=5;
  if (strcmp("11d",iconText.c_str())==0) iconNr=6;
  if (strcmp("11n",iconText.c_str())==0) iconNr=6;
  if (strcmp("13d",iconText.c_str())==0) iconNr=7;
  if (strcmp("13n",iconText.c_str())==0) iconNr=7;
  if (strcmp("50d",iconText.c_str())==0) iconNr=8;
  if (strcmp("50n",iconText.c_str())==0) iconNr=8;
  return iconNr;
}

void my_BMP::_hexDump(const char line[], size_t length) {
    for (size_t i = 0; i < length; i += 16) {
        Serial.printf("%08lX: ", (unsigned long)i);
        size_t j;
        for (j = i; j < i + 16 && j < length; ++j) {
            Serial.printf("%02X ", (unsigned char)line[j]);
        }
        for (; j < i + 16; ++j) {
            Serial.printf("   ");
        }
        for (j = i; j < i + 16 && j < length; ++j) {
            if (isprint((unsigned char)line[j])) {
                putchar(line[j]);
            } else {
                putchar('.');
            }
        }
        printf("\n");
    }
}
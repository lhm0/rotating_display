// =========================================================================================================================================
//                                                 Rotating Display RD40
//                                                    © Ludwin Monz
// License: Creative Commons Attribution - Non-Commercial - Share Alike (CC BY-NC-SA)
// you may use, adapt, share. If you share, "share alike".
// you may not use this software for commercial purposes 
// =========================================================================================================================================

#include "RD_40.h"
#include "RD_40trafo.h"

#include <Arduino.h>
#include <Wire.h>

// =========================================================================================================================================
//                                                      Constructor
// =========================================================================================================================================

RD_40::RD_40() {
//  brightness = 50;
}


// =========================================================================================================================================
//                                                      upload() method
// =========================================================================================================================================

void RD_40::upload(int _brightness) {                                                      // formerly send_lines()    
    setComplete=false;
    brightness=_brightness;

    char _i2cSendBuffer[16];
    int i;
    for (i=0; i<241; i++) {
      if (_line_change[i]!=0) {
        _i2cSendBuffer[0]=(unsigned char)(i);
        if (i<240) 
        {
          int j;
          for (j=0; j<5; j++) _i2cSendBuffer[j+1]=_line[j][i];   
        }
        else {
          _i2cSendBuffer[1]=(unsigned char)(brightness);
        }

        int conf=254; 
        while (conf!=i) {        
          Wire.beginTransmission(4);
          Wire.write(_i2cSendBuffer,6);
          Wire.endTransmission();

          Wire.requestFrom(4,1);    // request 2 bytes from peripheral device #4

          while (Wire.available()) {
            char a;
            a = Wire.read(); // receive a byte
            conf = (int)a;
          }
        }
      _line_change[i]=0;
      }
    }
    setComplete=true;
}

// =========================================================================================================================================
//                                                      displayBMP(unsigned char bmp[][14]) method
// =========================================================================================================================================

void RD_40::displayBMP(unsigned char bmp[][14]) {              // formerly trafo()
  int r,a;
  for (r=0; r<40; r++) {
    for (a=0; a<240; a++) {
      unsigned char x, y;
      x = pgm_read_byte(&trafo_x[r][a]);
      y = pgm_read_byte(&trafo_y[r][a]);
      if (_testBmpBit(bmp,x,y)) _setLineBit(r,a);
      else _clearLineBit(r,a);
    }
  }
  _check_changes();
}

// =========================================================================================================================================
//                                                      methods for testing and manipulating bitmap bits
// =========================================================================================================================================

bool RD_40::_testBmpBit(unsigned char bmp[][14], int x, int y) {
  int y_byte, y_bit;
  unsigned char y_test;
  y_byte=y/8;
  y_bit=y%8;
  y_test=1;
  y_test<<=y_bit;
  return (bmp[x][y_byte] & y_test);
}

void RD_40::_setBmpBit(unsigned char bmp[][14], int x, int y) {
  int y_byte, y_bit;
  unsigned char y_test;
  y_byte=y/8;
  y_bit=y%8;
  y_test=1;
  y_test<<=y_bit;
  bmp[x][y_byte]|=y_test;  
}

void RD_40::_clrBmpBit(unsigned char bmp[][14], int x, int y) {
  int y_byte, y_bit;
  unsigned char y_test;
  y_byte=y/8;
  y_bit=y%8;
  y_test=1;
  y_test<<=y_bit;;
  y_test=~(y_test);
  bmp[x][y_byte]&=y_test;    
}

// =========================================================================================================================================
//                                                      methods for manipulating line bits (=LEDs)
// =========================================================================================================================================

void RD_40::_setLineBit(int r, int a) {

// LED#   00 01 02 03 04 05 06 07 | 08 09 10 11 12 13 14 15 | 16 17 18 19 20 21 22 23 | 24 25 26 27 28 29 30 31 | 32 33 34 35 36 37 38 39
// byte#   0  3  0  3  0  3  0  3 |  0  3  0  3  0  3  0  3 |  1  4  1  4  1  4  1  4 |  1  4  1  4  1  4  1  4 |  2  2  2  2  2  2  2  2
// bit#    0  0  1  1  2  2  3  3 |  4  4  5  5  6  6  7  7 |  0  0  1  1  2  2  3  3 |  4  4  5  5  6  6  7  7 |  0  4  1  5  2  6  3  7

unsigned char lineByte[40]={0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};
unsigned char lineBit[40] ={0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x10, 0x02, 0x20, 0x04, 0x40, 0x08, 0x80};

  _line[lineByte[r]][a]|=lineBit[r];
}

void RD_40::_clearLineBit(int r, int a) {

// LED#   00 01 02 03 04 05 06 07 | 08 09 10 11 12 13 14 15 | 16 17 18 19 20 21 22 23 | 24 25 26 27 28 29 30 31 | 32 33 34 35 36 37 38 39
// byte#   0  3  0  3  0  3  0  3 |  0  3  0  3  0  3  0  3 |  1  4  1  4  1  4  1  4 |  1  4  1  4  1  4  1  4 |  2  2  2  2  2  2  2  2
// bit#    0  0  1  1  2  2  3  3 |  4  4  5  5  6  6  7  7 |  0  0  1  1  2  2  3  3 |  4  4  5  5  6  6  7  7 |  0  4  1  5  2  6  3  7

unsigned char lineByte[40]={0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};
unsigned char lineBit[40] ={0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x10, 0x02, 0x20, 0x04, 0x40, 0x08, 0x80};

  _line[lineByte[r]][a]&=~lineBit[r];
}

void RD_40::_check_changes() {
  int i,k,count=0;
  for (k=0; k<240; k++) {
    if ((_line[0][k]!=_line_prev[0][k])||(_line[1][k]!=_line_prev[1][k])||(_line[2][k]!=_line_prev[2][k])||(_line[3][k]!=_line_prev[3][k])||(_line[4][k]!=_line_prev[4][k])||sendAll) 
    {
      for (i=0; i<5; i++) _line_prev[i][k]=_line[i][k];
      _line_change[k]=1;
      count++;
    }
    else {
      _line_change[k]=0;
    }
  }
  _line_change[240]=1;
  sendAll=false;
}

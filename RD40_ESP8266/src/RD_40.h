// =========================================================================================================================================
//                                                 Rotating Display RD40
//                                                    © Ludwin Monz
// License: Creative Commons Attribution - Non-Commercial - Share Alike (CC BY-NC-SA)
// you may use, adapt, share. If you share, "share alike".
// you may not use this software for commercial purposes 
// =========================================================================================================================================


// ******************************************************************************************************************************************
//
//   This class provides the rotating display object. It consists of the display data (one bit per LED) and of methods for manipulating
//   the data and for transferring it to the display controller.
//
// ******************************************************************************************************************************************


#ifndef RD_40_H
#define RD_40_H

#include <Arduino.h>

class RD_40 {
  private:

    unsigned char _line[5][241];                                  // 5 bytes x 8 bits = 40 bits per line (40 LEDs); 241 lines
    unsigned char _line_prev[5][241] = {0};                       // the previous line pattern is saved. Only changed lines will be transferred to the display
    unsigned char _line_change[241] = {0};                        // if (line_change == 1)  => change, line needs to be transferred

    bool _testBmpBit(unsigned char bmp[][14], int x, int y);      // test bit of bitmap
    void _setBmpBit(unsigned char bmp[][14], int x, int y);       // set bit of bitmap
    void _clrBmpBit(unsigned char bmp[][14], int x, int y);       // clear bit of bitmap
    void _setLineBit(int r, int a);                               // set line bit
    void _clearLineBit(int r, int a);                             // clear line bit
    void _check_changes();                                        // check changes => compare line[][] with line_prev[][]

  public:

    int brightness;                                               // brightness level in %
    bool animation_mode=false;                                    // if true, the animation mode of the Arduino nano will be activated
    boolean sendAll=true;                                         // if true, all lines need to be sent
    boolean setComplete=true;                                     // used to control upload
  
    RD_40();                                                      // constructor

    void upload(int _brightness);                                 // uploads display image data and brightness setting to display controller
    void displayBMP(unsigned char bmp[][14]);                     // displays a bitmap

};

#endif

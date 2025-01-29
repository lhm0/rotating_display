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
    // Constructor: Initialize default values
    brightness = 50;  // Default brightness level
}

// =========================================================================================================================================
//                                                      upload() method
// =========================================================================================================================================

void RD_40::upload(int brightness) {
    // Upload display data to the display controller and set brightness
    setComplete = false;
    this->brightness = brightness;

    char i2cSendBuffer[16];  // Buffer for I2C communication

    // Send each line of display data
    for (int i = 0; i <= 240; i++) {
        int lineIndex = (i == 0) ? 240 : i - 1;  // Line 240 (parameters) is sent first

        if (_line_change[lineIndex] != 0) {
            i2cSendBuffer[0] = (unsigned char)(lineIndex);

            if (lineIndex < 240) {
                // Send display data for lines 0-239
                for (int j = 0; j < 5; j++) {
                    i2cSendBuffer[j + 1] = _line[j][lineIndex];
                }
            } else {
                // Send parameters for line 240 (brightness and animation mode)
                i2cSendBuffer[1] = (unsigned char)(brightness);
                i2cSendBuffer[2] = (animation_mode) ? 0xFF : 0x00;
            }

            // Send data via I2C and wait for confirmation
            int confirmation = 254;
            while (confirmation != lineIndex) {
                Wire.beginTransmission(4);
                Wire.write(i2cSendBuffer, 6);
                Wire.endTransmission();

                Wire.requestFrom(4, 1);  // Request confirmation from peripheral device #4

                while (Wire.available()) {
                    confirmation = Wire.read();  // Read confirmation byte
                }
            }

            _line_change[lineIndex] = 0;  // Mark line as sent
        }
    }

    setComplete = true;  // Mark upload as complete
}

// =========================================================================================================================================
//                                                      displayBMP() method
// =========================================================================================================================================

void RD_40::displayBMP(unsigned char bmp[][14]) {
    // Convert bitmap data to display line data
    for (int row = 0; row < 40; row++) {
        for (int angle = 0; angle < 240; angle++) {
            unsigned char x = pgm_read_byte(&trafo_x[row][angle]);
            unsigned char y = pgm_read_byte(&trafo_y[row][angle]);

            if (_testBmpBit(bmp, x, y)) {
                _setLineBit(row, angle);  // Set LED bit if bitmap bit is set
            } else {
                _clearLineBit(row, angle);  // Clear LED bit if bitmap bit is not set
            }
        }
    }

    _check_changes();  // Update changes in display data
}

// =========================================================================================================================================
//                                                      Bitmap Bit Manipulation Methods
// =========================================================================================================================================

bool RD_40::_testBmpBit(unsigned char bmp[][14], int x, int y) {
    // Test if a specific bit in the bitmap is set
    int byteIndex = y / 8;
    int bitIndex = y % 8;
    unsigned char bitMask = 1 << bitIndex;
    return (bmp[x][byteIndex] & bitMask);
}

void RD_40::_setBmpBit(unsigned char bmp[][14], int x, int y) {
    // Set a specific bit in the bitmap
    int byteIndex = y / 8;
    int bitIndex = y % 8;
    unsigned char bitMask = 1 << bitIndex;
    bmp[x][byteIndex] |= bitMask;
}

void RD_40::_clrBmpBit(unsigned char bmp[][14], int x, int y) {
    // Clear a specific bit in the bitmap
    int byteIndex = y / 8;
    int bitIndex = y % 8;
    unsigned char bitMask = ~(1 << bitIndex);
    bmp[x][byteIndex] &= bitMask;
}

// =========================================================================================================================================
//                                                      Line Bit Manipulation Methods
// =========================================================================================================================================

void RD_40::_setLineBit(int row, int angle) {
    // Set a specific LED bit in the display line data
    static const unsigned char lineByte[40] = {
        0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
        0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x01, 0x04, 0x01, 0x04,
        0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04,
        0x01, 0x04, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02
    };
    static const unsigned char lineBit[40] = {
        0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
        0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01, 0x02, 0x02,
        0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40,
        0x80, 0x80, 0x01, 0x10, 0x02, 0x20, 0x04, 0x40, 0x08, 0x80
    };

    _line[lineByte[row]][angle] |= lineBit[row];
}

void RD_40::_clearLineBit(int row, int angle) {
    // Clear a specific LED bit in the display line data
    static const unsigned char lineByte[40] = {
        0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
        0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x01, 0x04, 0x01, 0x04,
        0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04,
        0x01, 0x04, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02
    };
    static const unsigned char lineBit[40] = {
        0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
        0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01, 0x02, 0x02,
        0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40,
        0x80, 0x80, 0x01, 0x10, 0x02, 0x20, 0x04, 0x40, 0x08, 0x80
    };

    _line[lineByte[row]][angle] &= ~lineBit[row];
}

// =========================================================================================================================================
//                                                      _check_changes() method
// =========================================================================================================================================

void RD_40::_check_changes() {
    // Check for changes in display data and mark lines for update
    int changeCount = 0;

    for (int angle = 0; angle < 240; angle++) {
        bool hasChanged = false;

        for (int i = 0; i < 5; i++) {
            if (_line[i][angle] != _line_prev[i][angle] || sendAll) {
                hasChanged = true;
                _line_prev[i][angle] = _line[i][angle];  // Update previous line data
            }
        }

        if (hasChanged) {
            _line_change[angle] = 1;  // Mark line as changed
            changeCount++;
        } else {
            _line_change[angle] = 0;  // Mark line as unchanged
        }
    }

    _line_change[240] = 1;  // Always mark parameter line as changed
    sendAll = false;        // Reset sendAll flag
}
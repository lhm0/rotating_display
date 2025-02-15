// =========================================================================================================
//                                                 Rotating Display RD40
//                                                    © Ludwin Monz
// License: Creative Commons Attribution - Non-Commercial - Share Alike (CC BY-NC-SA)
// You may use, adapt, share. If you share, "share alike".
// You may not use this software for commercial purposes.
// =========================================================================================================

// This is a stripped-down version of the software with no web user interface.
// It displays digital time on a rotating LED display.

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <Wire.h>
#include "tables.h"

// Global Variables
tm localTime;                               // Structure to hold local time
uint8_t bitmap[110][14];                    // Bitmap with 110x112 pixels (110 columns, 14 rows * 8 bits)
uint8_t ledLines[5][241];                   // 240 LED lines with 5 bytes each (40 bits per line)

// Function Declarations
void initializeWiFi();
void configureTime();
void updateLocalTime();
void generateTimeBitmap();
void clearBitmap();
void print12x18(int alignment, const char text[], int xPos, int yPos);
void print10x15(int alignment, const char text[], int xPos, int yPos);
void transformBitmapToPolar();
bool isBitmapBitSet(int x, int y);
void setLedBit(int row, int angle);
void clearLedBit(int row, int angle);
void uploadToDisplay(int brightness);

// Setup and Loop
void setup() {
  Serial.begin(115200);                     // Initialize serial communication for debugging
  initializeWiFi();                         // Connect to WiFi
  configureTime();                          // Configure NTP server and time zone

  delay(500);

  // Initialize I2C communication
  Wire.begin(2, 0);                         // SDA = GPIO2, SCL = GPIO0
  Wire.setClock(200000L);                   // Set I2C clock speed to 200 kHz
}

void loop() {
  updateLocalTime();                        // Update local time
  String timeStr = String(localTime.tm_hour) + ":" + String(localTime.tm_min) + ":" + String(localTime.tm_sec);
  Serial.println(timeStr);                  // Print time to serial monitor

  generateTimeBitmap();                     // Generate bitmap for the current time
  transformBitmapToPolar();                 // Convert bitmap to polar coordinates
  uploadToDisplay(50);                      // Upload data to display with 50% brightness

  delay(1000);                              // Wait for 1 second
}

// Function Definitions

// Initialize WiFi Connection
void initializeWiFi() {
  const char* ssid = "PUT YOUR SSID HERE";
  const char* password = "PUT YOUR PASSWORD HERE";

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.println("IP address: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi connection failed...");
  }
}

// Configure NTP Server and Time Zone
void configureTime() {
  const char* timeZone = "CEST-1CET,M3.5.0/2:00:00,M10.5.0/3:00:00";
  const char* ntpServer = "pool.ntp.org";

  configTime(0, 0, ntpServer);              // Connect to NTP server with 0 TZ offset
  setenv("TZ", timeZone, 1);                // Set time zone
  tzset();
}

// Update Local Time
void updateLocalTime() {
  time_t now = time(nullptr);               // Get current timestamp
  localTime = *localtime(&now);             // Convert to local time
}

// Generate Bitmap for Current Time
void generateTimeBitmap() {
  char timeText[64];
  clearBitmap();                            // Clear the bitmap

  // Print time and date to bitmap
  sprintf(timeText, ":");
  print12x18(1, timeText, 40, 72);          // Print colon separator
  print12x18(1, timeText, 70, 72);          // Print colon separator

  sprintf(timeText, "%02d", localTime.tm_hour);
  print12x18(1, timeText, 25, 72);          // Print hours

  sprintf(timeText, "%02d", localTime.tm_min);
  print12x18(1, timeText, 55, 72);          // Print minutes

  sprintf(timeText, "%02d", localTime.tm_sec);
  print12x18(1, timeText, 85, 72);          // Print seconds

  sprintf(timeText, "%02d.%02d.%04d", localTime.tm_mday, localTime.tm_mon + 1, localTime.tm_year + 1900);
  print10x15(1, timeText, 55, 24);          // Print date (day.month)
}

// Clear Bitmap
void clearBitmap() {
  for (int x = 0; x < 110; x++) {
    for (int y = 0; y < 14; y++) {
      bitmap[x][y] = 0x00;                  // Clear each byte in the bitmap
    }
  }
}

// Print 12x18 Font Characters to Bitmap
void print12x18(int alignment, const char text[], int xPos, int yPos) {
  int textLength = strlen(text);
  int xOffset = 0;

  // Adjust xPos based on alignment
  switch (alignment) {
    case 1: // Center alignment
      xOffset = -6 * textLength;
      break;
    case 2: // Right alignment
      xOffset = -12 * textLength;
      break;
    default: // Left alignment (no adjustment)
      break;
  }

  xPos += xOffset;

  // Loop through each character in the text
  for (int charIndex = 0; charIndex < textLength; charIndex++) {
    for (int x = 0; x < 12; x++) {
      uint32_t charData = (uint32_t)pgm_read_dword(&chr_12x18[(int)text[charIndex] - 0x20][3 * x]);
      charData &= 0x03FFFF;                 // Ensure only 18 bits are used

      if ((xPos + x + 12 * charIndex) < 110) {
        charData << yPos%8;
        int offset = yPos/8;
        if (offset<14) bitmap[xPos+x+12*charIndex][offset] |= (charData >>  0) & 0xFF;
        if (offset<13) bitmap[xPos+x+12*charIndex][offset + 1] |= (charData >>  8) & 0xFF;
        if (offset<12) bitmap[xPos+x+12*charIndex][offset + 2] |= (charData >> 16) & 0xFF;
        if (offset<11) bitmap[xPos+x+12*charIndex][offset + 3] |= (charData >> 24) & 0xFF;
      }
    }
  }
}

// Print 10x15 Font Characters to Bitmap
void print10x15(int alignment, const char text[], int xPos, int yPos) { 
 
  int textLength = strlen(text);
  int xOffset = 0;

  // Adjust xPos based on alignment
  switch (alignment) {
    case 1: // Center alignment
      xOffset = -5 * textLength;
      break;
    case 2: // Right alignment
      xOffset = -10 * textLength;
      break;
    default: // Left alignment (no adjustment)
      break;
  }

  xPos += xOffset;

  // Loop through each character in the text
  for (int charIndex = 0; charIndex < textLength; charIndex++) {
    for (int x = 0; x < 10; x++) {
      uint32_t charData = (uint32_t)pgm_read_dword(&chr_10x15[(int)text[charIndex] - 0x20][2 * x]);
      if ((xPos + x + 10 * charIndex) < 110) {
        charData &= 0x7FFF;       // only use 15 bits
        charData << yPos%8;
        int offset = yPos/8;
        if ((xPos + x + 10 * charIndex) < 110) {
          if (offset<14) bitmap[xPos+x+10*charIndex][offset] |= (charData >>  0) & 0xFF;
          if (offset<13) bitmap[xPos+x+10*charIndex][offset + 1] |= (charData >>  8) & 0xFF;
          if (offset<12) bitmap[xPos+x+10*charIndex][offset + 2] |= (charData >> 16) & 0xFF;
          if (offset<11) bitmap[xPos+x+10*charIndex][offset + 3] |= (charData >> 24) & 0xFF;
        }
      }
    }
  }
}

// Transform Bitmap to Polar Coordinates
void transformBitmapToPolar() {
  for (int row = 0; row < 40; row++) {
    for (int angle = 0; angle < 240; angle++) {
      uint8_t x = pgm_read_byte(&trafo_x[row][angle]);
      uint8_t y = pgm_read_byte(&trafo_y[row][angle]);

      if (isBitmapBitSet(x, y)) {
        setLedBit(row, angle);              // Set LED bit if bitmap bit is set
      } else {
        clearLedBit(row, angle);            // Clear LED bit if bitmap bit is not set
      }
    }
  }
}

// Check if a Specific Bit in the Bitmap is Set
bool isBitmapBitSet(int x, int y) {
  int byteIndex = y / 8;
  int bitIndex = y % 8;
  return (bitmap[x][byteIndex] & (1 << bitIndex));
}

// Set a Specific LED Bit in the Display Line Data
void setLedBit(int row, int angle) {
  static const uint8_t lineByte[40] = {
        0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
        0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x01, 0x04, 0x01, 0x04,
        0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04,
        0x01, 0x04, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02
    };
  static const uint8_t lineBit[40] = {
        0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
        0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01, 0x02, 0x02,
        0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40,
        0x80, 0x80, 0x01, 0x10, 0x02, 0x20, 0x04, 0x40, 0x08, 0x80
    };

  ledLines[lineByte[row]][angle] |= lineBit[row];
}

// Clear a Specific LED Bit in the Display Line Data
void clearLedBit(int row, int angle) {
  static const uint8_t lineByte[40] = {
        0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
        0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x01, 0x04, 0x01, 0x04,
        0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04, 0x01, 0x04,
        0x01, 0x04, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02
    };
  static const uint8_t lineBit[40] = {
        0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
        0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01, 0x02, 0x02,
        0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40,
        0x80, 0x80, 0x01, 0x10, 0x02, 0x20, 0x04, 0x40, 0x08, 0x80
    };

  ledLines[lineByte[row]][angle] &= ~lineBit[row];
}

// Upload Display Data to the Display Controller
void uploadToDisplay(int brightness) {
  uint8_t i2cBuffer[16];                    // Buffer for I2C communication

  for (int i = 0; i <= 240; i++) {
    i2cBuffer[0] = (uint8_t)(i);

    if (i < 240) {
      // Send display data for lines 0-239
      for (int j = 0; j < 5; j++) {
        i2cBuffer[j + 1] = ledLines[j][i];
      }
    } else {
      // Send parameters for line 240 (brightness and animation mode)
      i2cBuffer[1] = (uint8_t)(brightness);
      i2cBuffer[2] = 0x00;                  // Turn animation mode OFF
    }

    // Send data via I2C and wait for confirmation
    int confirmation = 254;
    int tries = 10;
    while ((confirmation != i) && (tries > 0)) {
      tries--;
      Wire.beginTransmission(4);
      Wire.write(i2cBuffer, 6);
      Wire.endTransmission();

      Wire.requestFrom(4, 1);               // Request confirmation from peripheral device #4
      while (Wire.available()) {
        confirmation = Wire.read();         // Read confirmation byte
      }
    }
  }
}
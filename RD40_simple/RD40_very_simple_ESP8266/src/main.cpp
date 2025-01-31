// =========================================================================================================
//                                                 Rotating Display RD40
//                                                    © Ludwin Monz
// License: Creative Commons Attribution - Non-Commercial - Share Alike (CC BY-NC-SA)
// You may use, adapt, share. If you share, "share alike".
// You may not use this software for commercial purposes.
// =========================================================================================================

// This is a stripped-down version of the software with no web user interface.
// It displays digital time on a rotating LED display.
// There is no coordinate transformation, therefore letters are bend alnog the disk

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <Wire.h>
#include "tables.h"

// Global Variables
tm localTime;                               // Structure to hold local time
uint8_t ledLines[5][241];                   // 240 LED lines with 5 bytes each (40 bits per line)

// Function Declarations
void initializeWiFi();
void configureTime();
void updateLocalTime();
void generateTimeBitmap();
void clearLedLines();
void print12x18(int alignment, const char text[], int theta, int r);
void setLedBit(int row, int angle);
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
  clearLedLines();                            // Clear the ledLines[][] array

  // Print time and date to bitmap
  sprintf(timeText, "%02d:%02d:%02d", localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
  print12x18(1, timeText, 0, 20);          // Print time

  sprintf(timeText, "%02d.%02d.%04d", localTime.tm_mday, localTime.tm_mon + 1,localTime.tm_year + 1900);
  print12x18(1, timeText, 120, 0);          // Print date upside down
}

// Clear Bitmap
void clearLedLines() {
  for (int theta = 0; theta < 240; theta++) {
    for (int i = 0; i < 5; i++) {
      ledLines[i][theta] = 0x00;                  // Clear each byte of ledLines[][]
    }
  }
}

// Print 12x18 Font Characters to Bitmap
void print12x18(int alignment, const char text[], int theta, int r) {
  int textLength = strlen(text);
  int thetaOffset = 0;

  // Adjust xPos based on alignment
  switch (alignment) {
    case 1: // Center alignment
      thetaOffset = -6 * textLength;
      break;
    case 2: // Right alignment
      thetaOffset = -12 * textLength;
      break;
    default: // Left alignment (no adjustment)
      break;
  }

  theta = theta + thetaOffset +240;         // add 240 as result could be nagative, otherwise
  theta = theta%240;                        // correct for potential flip over

  // Loop through each character in the text
  for (int charIndex = 0; charIndex < textLength; charIndex++) {
    for (int t = 0; t < 12; t++) {          // the character consist of 12 columns
      uint32_t charData = (uint32_t)pgm_read_dword(&chr_12x18[(int)text[charIndex] - 0x20][3 * t]);
      charData &= 0x03FFFF;                 // Ensure only 18 bits are used

      int thetaColumn = theta + 12*charIndex + t;
      thetaColumn = thetaColumn%240;        // correct for potential flip over

      for (int bit = 0; bit < 18; bit++) {
        if (charData & (1 << bit)) {
          setLedBit(r+bit, thetaColumn);
        } 
      }
    }
  }
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
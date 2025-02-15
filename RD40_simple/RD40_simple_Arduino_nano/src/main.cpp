#include <math.h>
#include <string.h>
#include <SPI.h>
#include <Wire.h>

// Constants
#define LINE_COUNT 240
#define GEAR_IMAGE_ROWS 80
#define BRIGHTNESS_UPDATE_INTERVAL_FAST 50
#define BRIGHTNESS_UPDATE_INTERVAL_SLOW 300
#define ANIMATION_UPDATE_INTERVAL 20

// Pin Definitions
const int SPI_CLOCK_PIN = 13;       // SPI Clock
const int SPI_MISO_PIN = 12;        // SPI MISO (not used in this code)
const int SPI_MOSI_PIN = 11;        // SPI MOSI
const int G_ENABLE_PIN = 10;        // G enable pin for 595 shift register
const int LATCH_PIN = 9;            // Latch pin for SPI (connects to RCLK of 595)
const int HALL_SENSOR_PIN_1 = 8;    // Hall sensor input 1
const int HALL_SENSOR_PIN_2 = 2;    // Hall sensor input 2 (INT0)

// Global Variables
long totalTimePerTurn = 0;          // Total time per rotation
long linePeriod = 1200;             // Timer interrupt every 825 µs (1.2 kHz line frequency)
long newLinePeriod;                 // Updated line period
int brightness = 50;                // Brightness in %
bool updateBrightness = true;       // Flag to recalculate PWM setting
double R, S;                        // Variables for brightness linearization
long pwmSet;                        // PWM setting for brightness control

int lineCounter0 = 60;              // Counter for displayed lines (1/4 turn ahead of lineCounter1)
int lineCounter1 = 0;               // Counter for displayed lines
int lineCounter2 = 0;               // Counter for lines after trigger
int lineOffset = 168;               // Clock orientation (12 is up)
int lastReceivedLine = 255;         // Last received line index

uint8_t lineData[LINE_COUNT][5] = {0}; // Buffer for line data

long lastBrightnessUpdateTime;      // Timestamp for brightness update

// I2C Buffer
unsigned char i2cBuffer[16];        // Buffer for I2C received data
int receiveEventFlag = 0;           // Flag to indicate I2C receive event

// Function Declarations
void setBrightness(int brightness);
void receiveEvent(int howMany);
void receiveEventHandling();
void requestEvent();
void generateTestImage();

// Setup Function
void setup() {
  // Configure pins
  pinMode(SPI_CLOCK_PIN, OUTPUT);    // SPI Clock
  pinMode(SPI_MISO_PIN, INPUT);      // SPI MISO (not used in this code)
  pinMode(SPI_MOSI_PIN, OUTPUT);     // SPI MOSI
  pinMode(G_ENABLE_PIN, OUTPUT);     // G enable pin for 595
  pinMode(LATCH_PIN, OUTPUT);        // Latch pin for SPI
  pinMode(HALL_SENSOR_PIN_1, INPUT_PULLUP);  // Hall sensor input 1
  pinMode(HALL_SENSOR_PIN_2, INPUT_PULLUP);  // Hall sensor input 2 (INT0)

  // Enable INT0 as external interrupt (Hall sensor)
  EICRA |= (1 << ISC01);  // Set INT0 to trigger on falling edge
  EIMSK |= (1 << INT0);   // Enable INT0
  sei();                  // Enable global interrupts

  // Configure Timer1 for Fast PWM Mode
  noInterrupts();
  TCCR1A = B00100011;     // Clear OC1B on compare match, set OC1B at BOTTOM
  TCCR1B = B00011001;     // Fast PWM, no prescaling
  OCR1A = linePeriod;     // Set initial line period
  OCR1B = 62000;          // Initial brightness setting
  TIMSK1 = B00000001;     // Enable Timer1 Overflow Interrupt
  interrupts();

  // Initialize SPI
  SPI.begin();

  // Initialisiere I2C
  Wire.begin(4);                   // join I2C bus as slave for device #4
  Wire.onRequest(requestEvent);    // register request event
  Wire.onReceive(receiveEvent);    // register receive event

  generateTestImage();

  // Initialize timestamps
  lastBrightnessUpdateTime = millis();
}

// Main Loop
void loop() {
  // Handle I2C receive event
  if (receiveEventFlag == 1) {
    receiveEventHandling();
  }

  // Update brightness if needed
  long timeSinceLastBrightnessUpdate = millis() - lastBrightnessUpdateTime;
  if ((timeSinceLastBrightnessUpdate > BRIGHTNESS_UPDATE_INTERVAL_SLOW) ||
      (timeSinceLastBrightnessUpdate > BRIGHTNESS_UPDATE_INTERVAL_FAST && updateBrightness)) {
    lastBrightnessUpdateTime = millis();
    updateBrightness = false;
    setBrightness(brightness); // Recalculate and set brightness
  }
}

// Timer1 Overflow Interrupt Service Routine
ISR(TIMER1_OVF_vect) {
  totalTimePerTurn += linePeriod;

  // Send line data via SPI
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  digitalWrite(LATCH_PIN, LOW);
  SPI.transfer(lineData[lineCounter1][4]);
  SPI.transfer(lineData[lineCounter1][3]);
  SPI.transfer((lineData[lineCounter0][2] & 0x0F) + (lineData[lineCounter1][2] & 0xF0));
  SPI.transfer(lineData[lineCounter0][1]);
  SPI.transfer(lineData[lineCounter0][0]);
  digitalWrite(LATCH_PIN, HIGH);
  SPI.endTransaction();

  // Increment line counters
  lineCounter0 = (lineCounter0 + 1) % LINE_COUNT;
  lineCounter1 = (lineCounter1 + 1) % LINE_COUNT;
  lineCounter2++;
}

// Hall Sensor Interrupt Service Routine
ISR(INT0_vect) {
  totalTimePerTurn += ICR1; // Add time since last interrupt
  newLinePeriod = totalTimePerTurn / (LINE_COUNT - 1); // Calculate new line period

  // Update line period with smoothing
  if (labs(newLinePeriod - linePeriod) > 4) {
    linePeriod = newLinePeriod; // Large change: update immediately
    updateBrightness = true;    // Recalculate brightness
  } else {
    linePeriod = (7 * linePeriod + newLinePeriod) / 8; // Small change: smooth update
  }

  OCR1A = linePeriod - 1; // Update Timer1 compare register
  totalTimePerTurn = 0;   // Reset total time per turn

  // Reset line counters
  lineCounter1 = lineOffset;
  lineCounter0 = (lineOffset + 60) % LINE_COUNT;
  lineCounter2 = 0;

  // Clear interrupt flags
  TCNT1 = 0;      // Reset Timer1 counter
  TIFR1 |= 0x01;  // Clear Timer1 Overflow Flag
  EIFR |= 0x01;   // Clear INT0 Flag
}

// Set Brightness Function
void setBrightness(int brightness) {
  R = 30.1 / log10(linePeriod); // Calculate R-value for linearization
  pwmSet = linePeriod - pow(2, (brightness / R)) - 1; // Calculate PWM setting
  pwmSet = constrain(pwmSet, 10, linePeriod - 10);    // Constrain PWM setting
  OCR1B = pwmSet; // Update Timer1 compare register for brightness
}

// I2C Receive Event Handler
void receiveEvent(int howMany) {
  receiveEventFlag = 1; // Set flag to indicate I2C receive event
}

// I2C Receive Event Handling
void receiveEventHandling() {
  receiveEventFlag = 0; // Clear flag
  int messageCount = 0;

  // Read I2C data into buffer
  while (Wire.available()) {
    i2cBuffer[messageCount++] = Wire.read();
  }

  // Process received data
  if (messageCount == 6) {
    if (i2cBuffer[0] < LINE_COUNT) {
      // Update line data
      for (int j = 0; j < 5; j++) {
        lineData[i2cBuffer[0]][j] = i2cBuffer[j + 1];
      }
    } else if (i2cBuffer[0] == LINE_COUNT) {
      // Update brightness
      brightness = i2cBuffer[1];
    }
    lastReceivedLine = i2cBuffer[0]; // Update last received line index
  }
}

// I2C Request Event Handler
void requestEvent() {
  Wire.write(lastReceivedLine); // Send last received line index
}

// Generate test image
void generateTestImage() {
  for (int i=0; i<5; i++) {
    lineData[0][i] = 0xFF;
    lineData[60][i] = 0xFF;
    lineData[120][i] = 0xFF;
  }
  for (int i=0; i<5; i++) {
    if (i!=2) lineData[150][i] = 0xFF;
    else lineData[150][i] = 0x00;
    if (i!=2) lineData[210][i] = 0xFF;
    else lineData[210][i] = 0x00;
  }
}
//Include standard libraries
#include <math.h>
#include <string.h>

long tpt = 0;                 // total time per turn
long line_period = 1200;      // This results in timer interrupt every 6600*0.125µs = 825 µs
                              // (5 turns per second, 240 lines per turn, 1.2 kHz line frequency)
long new_line_period;
int brightness = 50;          // brightness in %
bool updateBrightness = true; // if true, pwm_set will be re_calculated in main loop
double R, S;                  // R-value for linearization of brigthness
long pwm_set;                 // PWM setting

int line_counter0 = 60;       // counts the displayed lines. line_counter0 is 1/4 turn ahead of line_counter1
int line_counter1 = 0;        //
int line_counter2 = 0;        // counts lines after trigger
int line_offset = 168;        // clock orientation (12 is up); depends on where the trigger LED is positioned
int line_rcvd = 255;          // last received line


char line[240][5]={0};

long ti;                      // time stamp for brightness update;
long ti1;                     // time stamp for animation update

bool animation_mode = true;   // if true, the animation will be displayed
int tet =0;                   // animation angle

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                              function declarations
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void set_brightness(int brightness);
void receiveEvent(int howmany);
void receiveEventHandling();
void requestEvent();

void draw_gear(int theta);

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                      S P I
//
#include <SPI.h>
  const int latchPin = 9;

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                      I 2 C
//
// The nano is an I2C Slave. request events and receive events trigger an event interrupt. In order 
// not to interfere with the time cricitical timer1 interrupts, the event interrupts only set a flag,
// which is checked in the main loop. Therefore, I2C communication (which takes a little while) is 
// executed in the main loop.


#include <Wire.h>
  int messageCount = 0;
  unsigned char i2cBuffer[16];             // buffer for received package
  int requestEventFlag = 0;
  int receiveEventFlag = 0;

 
void setup()
{
//  Serial.begin(115200);
//  Serial.println("nano POV:");

  delay(1000);              // wait a second before initializing the I/O lines. Reason is to give the ESP8266-01s enough time to start
                            // (i2c wires need to be tri-state while starting, otherwise ESP8266-01s might enter prograamming mode)
  
  // pin 13 = SPI Clck;        pin 12 = SPI MISO;              pin 11 = SPI MOSI;      pin 10 = G enable Pin 595;
  // pin 9 = latch RClck 595;  pin 8 = Hall trigger Input;     pin 2 = Hall trigger Input

  pinMode(13, OUTPUT);       // SPI Clck
  pinMode(12, INPUT);        // SPI MISO
  pinMode(11, OUTPUT);       // SPI MOSI
  pinMode(10, OUTPUT);       // G enable Pin 595
  pinMode(9, OUTPUT);        // latch RClck 595
  pinMode(8, INPUT_PULLUP);  // Hall trigger Input
  pinMode(2, INPUT_PULLUP);  // Hall trigger Input


  // enable INT0 as external interrupt
  EICRA |= (1 << ISC01);    // set INT0 to trigger on falling edge
  EIMSK |= (1 << INT0);     // Turns on INT0

  sei();                    // turn on interrupts

  // set Timer1 to Mode15: Fast PWM (TOP=OCR1A, udate OCR1A at Bottom)
  // COM1A1 = 1: clear OC1A on compare match, set OC1A on Botton
  // set pre scaler to 1

  noInterrupts();           // temporarily disable all interrupts
  TCCR1A = 0;               //
  TCCR1B = 0;               //

  TCCR1A = B00100011;       // COM1A1=0, COM1A0=0, COM1B1=1, COM1B0=0, x=0, x=0, WGM11=1, WGM10=1 ;    // Clear OC1B on compare match, Set OC1B at BOTTOM,
  TCCR1B = B00011001;       // ICNC1=0, ICES1=0, x=0, WGM13=1, WGM12=1, CS12=0, CS11=0, CS10=1
                            // ICES1 = 0: on falling edge on ICP1 (D8 Arduino): TCNT1 => ICR1 

  OCR1A = line_period;      // set Output Compare Register to 625.
  OCR1B = 62000;            // set brightness

  TIMSK1 = B00000001;       // enable Timer1 Overflow Interrupt
  interrupts();             // enable all interrupts

  SPI.begin();
/*
  Wire.begin(4);                   // join I2C bus as slave for device #4
  Wire.onRequest(requestEvent);    // register request event
  Wire.onReceive(receiveEvent);    // register receive event
*/
  ti1 = millis();
  ti = millis();
}

void loop()
{
  if (receiveEventFlag==1) receiveEventHandling();


  if ((animation_mode == true)&&((millis()-ti1)>20)) {
    tet++;
    if (tet>239) tet=0;
    draw_gear(tet);
    ti1 = millis();
  }

  long dt = millis()-ti;    
  if ((dt>300) || (dt>50)&updateBrightness ) {
    ti=millis();
    updateBrightness = false;
    set_brightness(brightness);  
  }
}

// ===============================================================================================
// line interrupt
// is triggered by the timer1 for every line
//
ISR(TIMER1_OVF_vect) {
    tpt = tpt + line_period;

    SPI.beginTransaction(SPISettings(10000000,MSBFIRST,SPI_MODE0));
    digitalWrite(latchPin, LOW);
    SPI.transfer(line[line_counter1][4]);
    SPI.transfer(line[line_counter1][3]);
    SPI.transfer((line[line_counter0][2] & 0x0F) + (line[line_counter1][2] & 0xF0));
    SPI.transfer(line[line_counter0][1]);
    SPI.transfer(line[line_counter0][0]);
    digitalWrite(latchPin, HIGH);
    SPI.endTransaction();

    line_counter0++;
    if (line_counter0 > 239) line_counter0 = 0;
    line_counter1++;
    if (line_counter1 > 239) line_counter1 = 0;
    line_counter2++;
}
  
  // ====================================================================================================
  // Hall interrupt
  // triggered by the Hall sensor, once per turn
  //

ISR(INT0_vect) {                              
  tpt = tpt + ICR1;         // ICR1 holds the timer1 count when the Hall sensor reacted
                            // tpt is now the total time per turn

  new_line_period = tpt/239;                    // caculate line period out og total time per turn
  if(labs(new_line_period-line_period)>4) {
      line_period=new_line_period;              // when motor is turned on, tpt changes very much from turn to turn.
      updateBrightness = true;                  // 
  } else {
      line_period=(7*line_period+new_line_period)/8;   // in this case correct line_period fully. Otherwise make small corrections, only.
  }
  OCR1A = line_period-1;                        // set OCR1A to new line_period

  tpt = 0;                                      // reset tpt
  line_counter1 = line_offset;                  // restart line_counter
  line_counter0 = line_offset + 60;
  if (line_counter0 >= 240) line_counter0 = line_counter0-240;
  line_counter2 = 0;
  
  TCNT1 = 0;                                    // restart timer1: set count register to 0
  TIFR1 |= 0x01;                                // clear interrupt flag OVR1.
  EIFR  |= 0x01;                                // clear interrupt flag INTF0.
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                       compute brightness setting
//

void set_brightness(int brightness) {
  R = 30.1 / (log10(line_period));                              // reference: https://diarmuid.ie/blog/pwm-exponential-led-fading-on-arduino-or-other-platforms
  pwm_set = line_period - pow (2, (brightness / R)) - 1;
  if (pwm_set < 10) pwm_set = 10;
  if (pwm_set > line_period - 10) pwm_set = line_period - 10;
  OCR1B = pwm_set;
}


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                            manage I2C events
//


// =======================================================================
// receiveEvent is triggered, when new data has been received.
// in order not to blick interrupts, the data is NOT processed in this interrupt routine.
// instead, a flag is set, which is checked in loop{}. The data will be processed in loop{}
//
void receiveEvent(int howmany) {
  receiveEventFlag = 1;
}

// ========================================================================
// this routine will be called from loop{}, once the receiveEventFlag has been set.
// The routine saves the received data to line[][]
//
void receiveEventHandling() {
  receiveEventFlag = 0;
  messageCount=0;  
  int i = 0;
  while (0<Wire.available()) {
    i2cBuffer[i]=Wire.read();
    i++;
    messageCount++;
  }
      
// store received data
//
  if (messageCount==6) {
    int j;
    if (i2cBuffer[0] <240) for (j=0; j<5; j++) line[i2cBuffer[0]][j]=i2cBuffer[j+1];
    if (i2cBuffer[0] == 240) {
      brightness = (int)i2cBuffer[1]; 
      if (i2cBuffer[2]==0) animation_mode = false;
      else animation_mode = true;
    }
    line_rcvd=(int)i2cBuffer[0];
  }
}

void requestEvent() {
    requestEventFlag = 0;
    char a=(char)line_rcvd;
    Wire.write (a);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// image of a gear. 
// 
//

const PROGMEM uint8_t g[80][5] = {
  {0xFF, 0xFF, 0x00, 0xFF, 0xFF}, {0xFF, 0xFF, 0x11, 0xFF, 0xFF}, {0xFF, 0xFF, 0x73, 0xFF, 0xFF}, {0xFF, 0xFF, 0x7F, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0x7F, 0xFF, 0xFF}, {0xFF, 0xFF, 0x73, 0xFF, 0xFF}, {0xFF, 0xFF, 0x11, 0xFF, 0xFF}, {0xFF, 0xFF, 0x00, 0xFF, 0xFF},
  {0x0F, 0xF0, 0x00, 0x07, 0xF8}, {0x07, 0xF0, 0x00, 0x07, 0xF0}, {0x07, 0xE0, 0x00, 0x03, 0xF0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0},
  {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x11, 0x03, 0xE0}, {0x03, 0xE0, 0x73, 0x03, 0xE0}, {0x03, 0xE0, 0x7F, 0x03, 0xE0}, {0x03, 0xE0, 0xFF, 0x03, 0xE0}, {0x03, 0xE0, 0xFF, 0x03, 0xE0}, {0x03, 0xE0, 0x7F, 0x03, 0xE0}, {0x03, 0xE0, 0x73, 0x03, 0xE0}, {0x03, 0xE0, 0x11, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0},
  {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0},
  {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x11, 0x03, 0xE0}, {0x03, 0xE0, 0x73, 0x03, 0xE0}, {0x03, 0xE0, 0x7F, 0x03, 0xE0}, {0x03, 0xE0, 0xFF, 0x03, 0xE0}, {0x03, 0xE0, 0xFF, 0x03, 0xE0}, {0x03, 0xE0, 0x7F, 0x03, 0xE0}, {0x03, 0xE0, 0x73, 0x03, 0xE0}, {0x03, 0xE0, 0x11, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0},
  {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0},
  {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x11, 0x03, 0xE0}, {0x03, 0xE0, 0x73, 0x03, 0xE0}, {0x03, 0xE0, 0x7F, 0x03, 0xE0}, {0x03, 0xE0, 0xFF, 0x03, 0xE0}, {0x03, 0xE0, 0xFF, 0x03, 0xE0}, {0x03, 0xE0, 0x7F, 0x03, 0xE0}, {0x03, 0xE0, 0x73, 0x03, 0xE0}, {0x03, 0xE0, 0x11, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0},
  {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x03, 0xE0, 0x00, 0x03, 0xE0}, {0x07, 0xF0, 0x00, 0x03, 0xF0}, {0x07, 0xF0, 0x00, 0x07, 0xF0}, {0x0F, 0xF0, 0x00, 0x07, 0xF8},
};

void draw_gear(int theta) {
  for (int i=0; i<240; i++) {
    int ii = theta%80;
    for (int j=0; j<5; j++) {
      line[i][j] = pgm_read_byte(&(g[ii][j]));
    }
    theta++;
    if (theta>239) theta = 0;
  }
}


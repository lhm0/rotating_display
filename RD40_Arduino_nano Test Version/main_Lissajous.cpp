//Include standard libraries
#include <math.h>
#include <string.h>

long tpt = 0;
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
void set_brightness(int brightness);

long int ti;

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                              function declarations
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void set_brightness(int brightness);
void receiveEvent(int howmany);
void receiveEventHandling();
void requestEvent();

void set_pixel_polar(int r, int theta); 
void set_pixel_car(int x, int y);
int trafo_r(int x, int y);
int trafo_t(int x, int y);
void lissajous(int o1, int o2);
void square(int x1, int y1, int x2, int y2);

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
  Serial.begin(115200);
  Serial.println("nano POV:");

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
                            // ICES1 = 0: bei fallender Flanke an ICP1 (D8 Arduino): TCNT1 => ICR1 

  OCR1A = line_period;      // set Output Compare Register to 625.
  OCR1B = 62000;            // set brightness

  TIMSK1 = B00000001;       // enable Timer1 Overflow Interrupt
  interrupts();             // enable all interrupts

  SPI.begin();

  Wire.begin(4);                   // join I2C bus as slave for device #4
  Wire.onRequest(requestEvent);    // register request event
  Wire.onReceive(receiveEvent);    // register receive event

  square (16,16,96,96);
  lissajous(3,2);

}

void loop()
{
//    if (requestEventFlag==1) requestEventHandling();
    if (receiveEventFlag==1) receiveEventHandling();
  
    long int dt = millis()-ti;    
    if ((dt>300) || (dt>50)&updateBrightness ) {
      ti=millis();
      updateBrightness = false;
      set_brightness(brightness);  
  }
}


ISR(TIMER1_OVF_vect) {
//    if (line_counter2<240) tpt = tpt + line_period; // stop adding, when trigger started
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
  
ISR(INT0_vect) {                              
  tpt = tpt + ICR1;         // ICR1 holds the timer1 count when the Hall sensor reacted
                            // tpt is now the total time per turn

  new_line_period = tpt/239;
  if(labs(new_line_period-line_period)>4) {
      line_period=new_line_period;             // when motor is turned on, tpt changes very much from turn to turn.
      updateBrightness = true;              // 
  } else {
      line_period=(7*line_period+new_line_period)/8;                               // in this case correct line_period fully. Otherwise make small corrections, only.
  }
  OCR1A = line_period-1;      // set OCR1A to new line_period

  tpt = 0;        // reset tpt
  line_counter1 = line_offset;  // restart line_counter
  line_counter0 = line_offset + 60;
  if (line_counter0 >= 240) line_counter0 = line_counter0-240;
  line_counter2 = 0;
  
  TCNT1 = 0;                // restart timer1: set count register to 0
  TIFR1 |= 0x01;            // clear interrupt flag OVR1.
  EIFR  |= 0x01;            // clear interrupt flag INTF0.
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void set_brightness(int brightness) {
  R = 30.1 / (log10(line_period));                              // reference: https://diarmuid.ie/blog/pwm-exponential-led-fading-on-arduino-or-other-platforms
  pwm_set = line_period - pow (2, (brightness / R)) - 1;
  if (pwm_set < 10) pwm_set = 10;
  if (pwm_set > line_period - 10) pwm_set = line_period - 10;
  OCR1B = pwm_set;
}

void receiveEvent(int howmany) {
  receiveEventFlag = 1;
}

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
    if (i2cBuffer[0] == 240) brightness = (int)i2cBuffer[1]; 
    line_rcvd=(int)i2cBuffer[0];
//    Serial.print("received line: ");
//    Serial.println ((int)i2cBuffer[0]);
  }
}

/*void requestEvent() {
  requestEventFlag = 1;
}
*/

void requestEvent() {
    requestEventFlag = 0;
    char a=(char)line_rcvd;
    Wire.write (a);
}

// ===================================================================================
// this routine sets a pixel in line[][]. The coordinates are given by the radius
// r (0...55) 
// and the angle 
// theta (0...239).
// r = 0 is the origin of the coordinate system in the center of the disk. The bits r=0...r=15 are omitted.
//
// LED  00 01 02 03 04 05 06 07   08 09 10 11 12 13 14 15   16 17 18 19 20 21 22 23    24 25 26 27 28 29 30 31 
// byte x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    0  3  0  3  0  3  0  3     0  3  0  3  0  3  0  3  
// bit  x  x  x  x  x  x  x  x    x  x  x  x  x  x  x  x    0  0  1  1  2  2  3  3     4  4  5  5  6  6  7  7 
//
//
// LED  32 33 34 35 36 37 38 39   40 41 42 43 44 45 46 47   48 49 50 51 52 53 54 55
// byte 1  4  1  4  1  4  1  4    1  4  1  4  1  4  1  4    2  2  2  2  2  2  2  2
// bit  0  0  1  1  2  2  3  3    4  4  5  5  6  6  7  7    0  4  1  5  2  6  3  7

uint8_t l_byte[40] = {0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 2, 2, 2, 2, 2, 2, 2, 2};
uint8_t l_bit[40] = 
{0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08,   0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x80,
 0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08,   0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 
 0x01, 0x10, 0x02, 0x20, 0x04, 0x40, 0x08, 0x80};

void set_pixel_polar(int r, int theta) {

  if ((r>15)&&(r<56)) {
    r=r-16;
    int byte = l_byte[r];
    int bit = l_bit[r];

    line[theta][l_byte[r]]= line[theta][l_byte[r]]|l_bit[r];
  }
}

void set_pixel_car(int x, int y) {
  set_pixel_polar(trafo_r(x, y), trafo_t(x, y));
}

void lissajous(int o1, int o2) {
  double delta = 2*3.14*o1/400;
  for (int i = 0; i<400; i++) {
    double x = 56 + 40 * sin(o1*i*delta);
    double y = 56 + 40 * sin(o2*i*delta);
    set_pixel_polar(trafo_r(x,y), trafo_t(x,y));
  }
}

void square(int x1, int y1, int x2, int y2) {
  
    if (x2<x1) {
      int r = x1;
      x1 = x2;
      x2 = r;
    }
    if (y2<y1) {
      int r = y1;
      y1 = y2;
      y2 = r;
    }
    for (int x = x1; x<x2; x++) set_pixel_car(x, y1);
    for (int x = x1; x<x2; x++) set_pixel_car(x, y2);
    for (int y = y1; y<y2; y++) set_pixel_car(x1, y);
    for (int y = y1; y<y2; y++) set_pixel_car(x2, y);
    
}

int trafo_r(int x, int y) {
  x=x-56;
  y=y-56;
  double r=sqrt(x*x+y*y);
  return (int)r;
}

int trafo_t(int x, int y) {
  x=x-56;
  y=y-56;
  double theta = 120./3.14*atan((double)x/(double)y);
  if ((x>=0) && (y<0)) theta = 120.+theta;
  if ((x<0) && (y<0)) theta = 120.+theta;
  if ((x<0) && (y>=0)) theta = 240.+theta;

  return (int)theta;
}
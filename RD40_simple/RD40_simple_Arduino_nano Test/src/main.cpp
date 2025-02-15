#include <avr/interrupt.h>
#include <SPI.h>

const int interruptPin = 2;  // GPIO2 als Interrupt-Pin
const int LATCH_PIN = 9;
const int G_ENABLE_PIN = 10;
volatile bool triggerFlag = false;

void sendData(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4);

void handleInterrupt() {
    triggerFlag = true;
}

void setup() {
    pinMode(interruptPin, INPUT_PULLUP); // Setzt GPIO2 als Eingang mit Pullup-Widerstand
    pinMode(LED_BUILTIN, OUTPUT);        // Setzt die interne LED als Ausgang
    attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, RISING); // Interrupt bei steigender Flanke
    sei(); // Globale Interrupts aktivieren

    SPI.begin(); // SPI-Schnittstelle aktivieren
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(G_ENABLE_PIN, OUTPUT);
    digitalWrite(LATCH_PIN, HIGH);
    digitalWrite(G_ENABLE_PIN, HIGH);
}

void loop() {
  sendData(0xFF, 0xFF, 0x0F, 0x00, 0x00);
  digitalWrite(G_ENABLE_PIN, LOW);        // LEDs on
  delay(100);                             // delay
  digitalWrite(G_ENABLE_PIN, HIGH);       // LEDs off

  sendData(0x00, 0x00, 0xF0, 0xFF, 0xFF);
  digitalWrite(G_ENABLE_PIN, LOW);        // LEDs on
  delay(100);                             // delay
  digitalWrite(G_ENABLE_PIN, HIGH);       // LEDs off
}

void sendData(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4) {
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(LATCH_PIN, LOW);
  SPI.transfer(byte0);
  SPI.transfer(byte1);
  SPI.transfer(byte2);
  SPI.transfer(byte3);
  SPI.transfer(byte4);
  digitalWrite(LATCH_PIN, HIGH);
}

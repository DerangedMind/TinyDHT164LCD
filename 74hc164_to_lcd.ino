/*
    Connect Arduino to LCD Display with 74HC164 Shift Register
    
    Inspired by: http://www.bristolwatch.com/arduino/arduino4a.html 
    Lewis Loflin - lewis@bvu.net

    Hd44780 display commands:
        0x0c = initiate display with cursor off
        0x0f = initiate display with blinking cursor
        0x01 = clear display fills display with spaces (0x20).
        0x02 = HOME returns to line one first character
        0x38 = 2 lines X 16 char 8 bits mode. Defaults to 1 line mode.
        0x10 = cursor left
        0x14 = cursor right
        0x18 = Shifts entire display left
        0x1c = Shifts entire display right 
        
        One can also go to a specific location.
        sendCommand(0x80); // begin on 1st line
        sendCommand(0x80 + 0x40); // begin on 2nd line 
        
        sendCommand(0x38); // setup for 2 lines
        sendCommand(0x0F); // blinking cursor
        
        sendCommand(0x02); // home
        sendCommand(0x01); // clear
*/

#include "MiniDHT.h"
#include <avr/power.h>      // needed to up clock to 16 MHz on 5v Trinket

#define SDA 0  // Serial Data      Pin 1 74HC164
#define CLK 2  // Clock            Pin 8 74HC164
#define RS 4   // Register Select  Pin 4 LCD - { H:Data , L:Instruction }
#define E 3    // Enable           Pin 6 LCD

#define Line1 0x80         // LCD (row 0, col 0)
#define Line2 0x80 + 0x40  // LCD (row 1, col 0)

MiniDHT dht(1, DHT11);

void setup() {
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1); // 5V Trinket: run at 16 MHz
  dht.begin();

  pinMode(SDA, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(RS, OUTPUT);
  pinMode(E, OUTPUT);

  digitalWrite(CLK, LOW);
  digitalWrite(RS, LOW);  // LCD in command mode default
  digitalWrite(E, HIGH);

  initLCD();
}

void initLCD() {
  sendCommand(0x38);  // setup for 2 lines, default is 1 line
  sendCommand(0x0C);  // display on, no cursor
  clearDisplay();
}

void sendCommand(uint8_t val) {
  serialWrite(val);       // send byte to 74164
  digitalWrite(RS, LOW);  // make sure RS in Com mode
  pulseOut(E);
}

// inverts state of pin, delays, then reverts state back
void pulseOut(byte pin) {
  byte pinValue = digitalRead(pin);
  delayMicroseconds(10);
  digitalWrite(pin, !pinValue);
  delayMicroseconds(10);
  digitalWrite(pin, pinValue);
}

// writes serial data to 74HC164
void serialWrite(byte val) {      
  for (int j = 1; j <= 8; j++) {  // shift out MSB first
    byte temp = val & B10000000;  // MSB out first 
    if (temp == 0x80) digitalWrite(SDA, HIGH); 
    else digitalWrite(SDA, LOW);
    
    pulseOut(CLK);
    val = val << 1;  // shift one place left
  }                  // next j
}

void clearDisplay() {
  sendCommand(0x01);  // clear
  sendCommand(0x02);  // home
}

}

double getSVP(double t) {
    return 610.78 * exp((t / (t + 287.3)) * 17.2694);
}

void displaySensorData() {
  // char tempString[4];
  float t = dht.readTemperature();
  // dtostrf(t, -4, 1, tempString);
  
  delayMicroseconds(1000);
  sendCommand(Line1); // Go to Line1
  writeChar(0x54);       // "T"
  writeChar(0x20);       // " "
  writeFloat(t, 1);      // "21.7"
  writeChar(0b11011111); // "˚"
  writeChar(0b01000011); // "C"
  writeChar(0x20);       // " "
  
  // char humidityString[3];
  float h = dht.readHumidity();
  // dtostrf(h, -3, 1, humidityString);

  writeChar(0b01001000); // "H"
  writeChar(0x20);       // " "
  writeFloat(h, 1);      // "35.1"
  writeChar(0b00100101); // "%"


  float avpd = getSVP(t) * (1 - h / 100);
  float lvpd = (getSVP(t - 2) - (getSVP(t) * h / 100)) / 1000;

  sendCommand(Line2);
  writeChar(0b01001100); // "L"
  writeChar(0b01010110); // "V"
  writeChar(0b01010000); // "P"
  writeChar(0b01000100); // "D"
  writeChar(0x20);       // " "
  writeFloat(lvpd, 2);   // "1.22"
  writeChar(0x20);       // " "
  writeChar(0b01101011); // "k"
  writeChar(0b01010000); // "P"
  writeChar(0b01100001); // "a"
}

void writeChar(byte value) {
  serialWrite(value);
  digitalWrite(RS, HIGH);
  pulseOut(E);
  digitalWrite(RS, LOW);
}

// change a float to ASCII
// dtostrf(floatVar, minWidth, decimalPrecision, charBuf);
void writeFloat(float value, int precision) {
  char numberString[4];
  dtostrf(value, -3, precision, numberString);
  writeToLCD(numberString);
}

// writes *char[] to LCD, location optional
void writeToLCD(char *s) {
  delayMicroseconds(1000);
  while (*s) writeChar(*(s++));
}

void writeToLCD(char *s, int location) {
  delayMicroseconds(1000);
  sendCommand(location);  // where to begin
  while (*s) writeChar(*(s++));
}

void loop() {
  displaySensorData();
  delay(12000);
}

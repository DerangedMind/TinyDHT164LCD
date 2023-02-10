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

// void writeHelloWorld() {
//   char helloString[] = "Hello world!";
//   writeToLCD(helloString, Line1);
// }

// void displayReading(char label[], char metric[], float value, int location, int width, int floats) {
//   char tempString[width];
//     dtostrf(value, -width, floats, tempString);

//     writeToLCD(label, location);
//     writeToLCD(tempString, location + 0x02);
//     writeToLCD(metric, location + 0x02 + 0x04);
// }

// void writeTemperature(bool asFahrenheit = false) {
//   char label[] = "T ";
//   char metric[] = "\337C";

//   float sensorTemp = dht.readTemperature(asFahrenheit);
//   // if (sensorTemp == -999) {
//   //   char errorMessage[] = "ReadErr: -999";
//   //   writeToLCD(errorMessage, Line1);
//   // } else {
//     char tempString[4];
//     dtostrf(sensorTemp, -4, 1, tempString);

//     writeToLCD(label, Line1);
//     writeToLCD(tempString, Line1 + 2);
//     writeToLCD(metric, Line1 + 2 + 3);
//   // }
// }

// void writeHumidity() {
//   char label[] = "H ";
//   char metric[] = "%";

//   float sensorHum = dht.readHumidity();
  
//   // if (sensorHum == -99) {
//   //   char errorMessage[] = "ReadErr: -99";
//   //   writeToLCD(errorMessage, Line1);
//   // } else {
//     char humidityString[3];
//     dtostrf(sensorHum, -3, 1, humidityString);

//     writeToLCD(label, Line1 + 0x08);
//     writeToLCD(humidityString, Line1 + 0x08 + 0x02);
//     writeToLCD(metric, Line1 + 0x08 + 0x02 + 0x04);
//   // }
// }

int pow(int base, int exp) {
    if(exp < 0) return -1;

    int result = 1;
    while (exp)
    {
        if (exp & 1) result *= base;
        exp >>= 1;
        base *= base;
    }

    return result;
}


double getSVP(double t) {
    double expo = pow(2.71828, (t / (t + 287.3)) * 17.2694);
    return 610.78 * expo;
}

void writeAVPD() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  char tLabel[] = "T ";
  char tMetric[] = "\337C";
  char tempString[4];
  dtostrf(t, -4, 1, tempString);

  writeToLCD(tLabel, Line1);
  writeToLCD(tempString, Line1 + 2);
  writeToLCD(tMetric, Line1 + 2 + 3);

  char hLabel[] = "H ";
  char hMetric[] = "%";
  char humidityString[3];
  dtostrf(h, -3, 1, humidityString);

  writeToLCD(hLabel, Line1 + 0x08);
  writeToLCD(humidityString, Line1 + 0x08 + 0x02);
  writeToLCD(hMetric, Line1 + 0x08 + 0x02 + 0x04);

  float avpd = getSVP(t) * (1 - h / 100);
  float lvpd = (getSVP(t - 2) - (getSVP(t) * h / 100)) / 1000;

  char label[] = "LVPD";
  char metric[] = "kPa";
  char value[4];
  dtostrf(lvpd, -4, 2, value);

  writeToLCD(label, Line2);
  writeToLCD(value, Line2 + 0x04);

}

void writeChar(byte value) {
  serialWrite(value);
  digitalWrite(RS, HIGH);
  pulseOut(E);
  digitalWrite(RS, LOW);
}

// Below we pass a pointer to array1[0].
void writeToLCD(char *s, int location) {
  delayMicroseconds(1000);
  sendCommand(location);  // where to begin
  while (*s) writeChar(*(s++));
}

void loop() {
  // clearDisplay();
  // writeHelloWorld();
  // displayReading("T", "\337C", dht.readTemperature(), Line1, 4, 1);
  // displayReading("H", "%", dht.readHumidity(), Line1 + 0x07, 4, 1);
  // writeTemperature();
  // writeHumidity();
  writeAVPD();
  delay(12500);
}

// change a float to ASCII
// dtostrf(floatVar, minWidth, decimalPadding, charBuf);
// float is number of decimal point values to display
// void writeFloat(float value, int location) {
//   char numberString[10];
//   dtostrf(value, 5, 1, numberString);
//   writeToLCD(numberString, location);
// }

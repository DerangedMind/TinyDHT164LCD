# Trinket Sensor using DHT11 to 16x2 LCD


## Project Summary

Temperature and humidity readings from the DHT11 sensor are used to calculate vapor density. Data is displayed on a 16x2 LCD by passing serial data through a 74HC164 shift register. This allows the Adafruit Trinket (ATtiny85, 5v) to connect to an external LCD without requiring an I2C backpack, taking full advantage of all available GPIO pins, with one to spare for the DHT11 Temperature/Humidity sensor.

## Limitations

Due to the limited program storage space available on the device (5kB), using the provided libraries was not an option as it would not fit on the device. While Adafruit offers a (TinyDHT)[https://github.com/adafruit/TinyDHT] package that reduces file size, it does so by sacrificing accuracy (only `int` values are saved, `float` are discarded). 

Fortunately, I had based my project off of (Lewis Loflin's LCD Driver Using 74HC164)[https://www.bristolwatch.com/arduino/arduino4a.html]. The 74HC164 is an 8-bit Serial-In, Parallel-Out Shift Register, capable of taking one serial input and converting it into 8 digital outputs. This handled most of the heavy lifting and allowed me to pass byte commands to the display rather than hardcoding strings which take up significantly more program storage. 

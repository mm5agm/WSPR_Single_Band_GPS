/*****************************************************************************************************************
***                    Author Colin Campbell MM5AGM                 mm5agm@outlook.com                         ***
*** This program is free software: you can redistribute it and/or modify it under the terms of the GNU         ***
*** General Public License as published by the Free Software Foundation, either version 3 of the License,      ***
*** or (at your option) any later version.                                                                     ***
***                                                                                                            ***
*** This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without          ***
*** even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                          ***
*** See the GNU General Public License for more details.                                                       ***
******************************************************************************************************************/

/*****************************************************************************************************************
*** This is the second of a series of programs that culminates in a WSPR beacon transmitter. Each program      ***
*** builds on the previous one by adding 1 component or more code. This program scans the I2C bus and reports  ***
*** the addresses, hexadecimal, of any devices found.                                                          ***
*** Hardware required = ESP32                                                                                  ***
******************************************************************************************************************/

#include <Wire.h>

/**************************************************************************
***   The setup function runs just once at the start of the program     ***
***************************************************************************/
void setup() {
  Wire.begin();
  Serial.begin(9600);
  while (!Serial) {
    // wait for serial port to connect. Needed for native USB port only
  }
}
/**************************************************************************
***   The loop() function runs continuously after setup.                ***
***   When it gets to the end of the loop it goes back to the beginning ***
***   of loop()                                                         ***
***************************************************************************/
void loop() {
  /* The I2C address is a 7-bit address so can be from 0 to 127 however 
  addresses 0 to 7, and 120 to 127 are reserved.
  */

  int startAddress = 8;
  int endAddress = 119;
  int error, address, numDevices;

  Serial.println("Scanning...");
  numDevices = 0;
  for (address = startAddress; address <= endAddress ; address++) {  // valid I2C addresses are from 8 to 119
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      if (address < 16) {
        Serial.print("I2C device found at address 0x");
        Serial.print("0");
        Serial.print(address, HEX);
        Serial.println(" !");
        numDevices++;
      } else {
        Serial.print("I2C device found at address 0x");
        Serial.print(address, HEX);
        Serial.println(" !");
        numDevices++;
      }
      delay(1000);
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
        Serial.println(address, HEX);
      } else {
        Serial.println(address, HEX);
      }
    }
  }
}

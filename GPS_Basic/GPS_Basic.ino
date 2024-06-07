/*****************************************************************************************************************
***                     Author Colin Campbell MM5AGM             mm5agm@outlook.com                            ***
*** This program is free software: you can redistribute it and/or modify it under the terms of the GNU         ***
*** General Public License as published by the Free Software Foundation, either version 3 of the License,      ***
*** or (at your option) any later version.                                                                     ***
***                                                                                                            ***
*** This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without          ***
*** even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                          ***
*** See the GNU General Public License for more details.                                                       ***
******************************************************************************************************************/

/*****************************************************************************************************************
*** This is the first of a series of programs that culminates in a WSPR beacon transmitter. Each program       ***
*** builds on the previous one by adding 1 component or more code. This program gets the date and time from    ***
*** a GPS module. There is no formatting of the date and time, that will be done in a later program.           ***
*** program. The GPS library is TinyGPSPlus and is available from the Arduino Library Manager                  ***
*** Hardware required = ESP32 , GPS module i used was a Neo 6M                                                 ***
*** In the Arduino IDE, in File/Preferences, fill in the “Additional boards manager URLs” with                 ***
***                 https://espressif.github.io/arduino-esp32/package_esp32_index.json                         ***
******************************************************************************************************************/
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

HardwareSerial gpsPort(2);

TinyGPSPlus gps;
#define RX_PIN 16                      // ESP32 Pin connected to the TX of the GPS module
#define TX_PIN 17                      // ESP32 Pin connected to the RX of the GPS module
static const uint32_t GPSBaud = 9600;  // set to your GPS baud rate. Some are 4800, some are 9600

/******************************[ smartDelay ] ************************************************
*** allow the gps to be updated while having a delay in the program                        ***    
**********************************************************************************************/     
static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (gpsPort.available())
      gps.encode(gpsPort.read());
  } while (millis() - start < ms);
}

/************************************[ initialiseGPS ] **************************************
***  initiaalise the GPS module and wait till a valid set of data is received             ***
*********************************************************************************************/
void initialiseGPS() {
  gpsPort.begin(GPSBaud, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial.println("Waiting for GPS to find satellites");
  gps.encode(gpsPort.read());
   while (!gps.location.isValid()) {
    gps.encode(gpsPort.read());
    Serial.print("*");
    smartDelay(100);
  }
}
/*****************************[ serialPrintTime ]******************************************
*** show the time on the serial monitor                                                 *** 
*********************************************************************************************/
void serialPrintTime() {
  Serial.print(gps.time.hour());
  Serial.print(":");
  Serial.print(gps.time.minute());
  Serial.print(":");
  Serial.print(gps.time.second());
  Serial.println();
}
/*****************************[ serialPrintDate ]******************************************
*** show the date on the serial monitor                                                 *** 
*******************************************************************************************/
void serialPrintDate() {
  Serial.print(" Year=");
  Serial.print(gps.date.year());
  Serial.print(" Month=");
  Serial.print(gps.date.month());
  Serial.print(" Day=");
  Serial.println(gps.date.day());
}
/*******************************************************************************************/

void setup() {
  Serial.begin(115200);
  delay(1000);  // give the port time to open
  initialiseGPS();
  Serial.println();
  Serial.print("Got Fix. Number Of Satellites = ");  // if we get to this line the GPS module is communicating with the ESP32
  Serial.println(gps.satellites.value());
}
/*******************************************************************************************/

void loop() {
  while (gpsPort.available() > 0)
    gps.encode(gpsPort.read());
  if (gps.date.isUpdated()) {
    serialPrintDate();
    Serial.print("Number Of Satellites =");
    Serial.println(gps.satellites.value());
  }
  if (gps.time.isUpdated()) {
    serialPrintTime();
  }
}
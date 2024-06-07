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
*** This is the third in a series of programs that culminates in a WSPR beacon transmitter. Each program      ***
*** builds on the previous one by adding 1 component or more code. This program gets the date, time, and       ***
*** location from GPS and displays this in the serial monitor and on the OLED display                          ***
*** Note that the displayed time will be slightly behind the real time because of the delay(1000) in the loop()***
*** The Time library has days 1 to 7, with 1 = Sunday and 7 = Saturday                                         ***
*** The OLED library is Adafruit_SSD1306 which also requires Adafruit_GFX. Both are available from the Arduino ***
*** library manager                                                                                            *** 
******************************************************************************************************************/

#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <Adafruit_GFX.h>      //  Adafruit 1.11.9
#include <Adafruit_SSD1306.h>  //  Adafruit 2.5.9
#include <TimeLib.h>           //  https://github.com/PaulStoffregen/Time
//OLED Display
#define SCREEN_WIDTH 128                                                         // OLED display width, in pixels
#define SCREEN_HEIGHT 64                                                         // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);                // create an instance of the SSD1306
const char* weekDays[] = { "Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat" };  //RTC weekDays[0] = "Sun",  weekDays[6] = "Sat"

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
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
    smartDelay(200);
  }
}

/****************************[serialPadZero] ******************************************************
***       print a "0" in front of a single digit number in the Serial Monitor Output            ***
***************************************************************************************************/
void serialPadZero(int aNumber) {
  if (aNumber < 10) {
    Serial.print("0");
    Serial.print(aNumber);
  } else {
    Serial.print(aNumber);
  }
}
/***************************[displayPadZero]*************************************************
*** displayPadZero - print a "0" in front of a single digit number OLED display           ***
*** As I want to use this inside other functions, I can't declare it as "void" so I       ***
*** declare it as returning a pointer to a character and send back ""                     ***
*********************************************************************************************/
void displayPadZero(int aNumber) {
  if (aNumber < 10) {
    display.print("0");
    display.print(aNumber);
  } else {
    display.print(aNumber);
  }
}

/***************************************************************************************
***                          Initialse the OLED                                      ***
****************************************************************************************/
void initialiseDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address found in I2C_Scanner.ino = 0x3C
    Serial.println("SSD1306 allocation failed");     // if the display isn't found you can't write an error message on it
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.display();  // you need display.display() or you don't update the display
  }
}
/***************************************************************************************
***  This is the data I want to see all the time.                                    ***
***  1st line shows ssid, 2nd I.P address, 3rd current time (with slight delay)      ***
****************************************************************************************/
void mainScreen() {
  display.clearDisplay();  // connected at this point
  display.setCursor(0, 10);
    display.print("Time is ");
  
  displayPadZero(gps.time.hour());
  display.print(':');
  displayPadZero(gps.time.minute());
  display.print(':');
  displayPadZero(gps.time.second());
  display.println();
 //  display.setCursor(1, 10);
  //   display.print("Date is ");
    display.print(weekDays[weekday()]);
    display.print("  ");
    displayPadZero(gps.date.day());
     display.print('/');
      displayPadZero(gps.date.month());
       display.print('/');
  displayPadZero(gps.date.year());
   display.println();
 //  display.print(weekDays[gps.date.day()]);
   display.println();
 // display.setCursor(3, 10);
 display.print(gps.location.lat(),5);
 display.print("   ");
display.print(gps.location.lng(),5);
 display.println();

  display.display();
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
Serial.println(weekDays[weekday()]);
 // Serial.println(weekday());
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
  Serial.println("Starting Up");
   initialiseDisplay();
 initialiseGPS();
  Serial.println();
  Serial.print("Got Fix. Number Of Satellites = ");  // if we get to this line the GPS module is communicating with the ESP32
  Serial.println(gps.satellites.value());
 
}
/**************************************************************************
***   The loop() function runs continuously after setup.                ***
***   When it gets to the end of the loop it goes back to the beginning ***
***************************************************************************/
void loop() {
  mainScreen();
   delay(1000);                 //wait 1 second before getting the time from the GPS, this will mean our output is a bit behind real time
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
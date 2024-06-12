/*****************************************************************************************************************
***                         Author Colin Campbell MM5AGM         mm5agm@outlook.com                            ***                                                                        ***
*** This program is free software: you can redistribute it and/or modify it under the terms of the GNU         ***
*** General Public License as published by the Free Software Foundation, either version 3 of the License,      ***
*** or (at your option) any later version.                                                                     ***
***                                                                                                            ***
*** This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without          ***
*** even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                          ***
*** See the GNU General Public License for more details.                                                       ***
******************************************************************************************************************/
/*****************************************************************************************************************
** This is GPS version of my single band WSPR beacon transmitter.                                               **
** The original version got the time and date from an NTP server. This version uses GPS                         **
** It's a work in progress and may be changed.                                                                  **
** You will need to change the callsign, si5351 calibration factor, and select the frequency to TX.             ** 
**  All times are UTC                                                                                           **                                                                                                                **
** Hardware required = ESP32, si5351 square wave generator, GPS module , SSD1306 0.96inch OLED                  **
** and a low pass filter for the band you are using                                                             **
** In Arduino IDE open the option “File/Preferences” and fill in the “Additional boards manager URLs” with      **
**  https://espressif.github.io/arduino-esp32/package_esp32_index.json                                          **
**                                                                                                              **
** GPS library is TinyGPSPlus                                               - install from Arduino IDE          **
** OLED library is Adafruit_SSD1306 which also requires Adafruit_GFX.h      - install both from Arduino IDE     **
** si5351 and WSPR encoding libraries are by Jason Milldrum                 - install both from Arduino IDE     **
** ESP32 - My multi band extension of this sketch uses the library "Sunset" and that requires a 32 bit FPU      **
** hence ESP32. ESP32 comes in 30 and 38 pin varieties. Tested with both varieties                              **
**                                                                                                              **
** si5351 square wave generator Clock0 is used as WSPR output.                                                  **
** si5351, and OLED SSD1306, conected via I2C bus on GPIO21 (SDA) and pin GPIO22 (SCL)                          ** 
** GPS TX connected to ESP32 RX(2) GPIO - My GPS is set to 9600 baud                                            **
******************************************************************************************************************/
/*  
  WSPR signals are 6 Hz wide.   
 The WSPR software will decode 100 Hz either side of the Centre Frequency, which gives a total receive bandwidth
 of only 200 Hz.
 The following dial frequencies are in use for WSPR. We need to add 1500Hz to the dial frequency to get the transmit frequency:
 TX centre Frequency = USB Dial Frequency + 1.5 KHz
 Dial Frequency           TX Frequency
80m: 3.568600 MHz         3,570.100 kHz  +/- 100 Hz
40m: 7.038600 MHz         7,040.100 kHz  +/- 100 Hz
30m: 10.138700 MHz        10,140.200 kHz +/- 100 Hz
20m: 14.095600 MHz        14,097.100 kHz +/- 100 Hz 
17m: 18.104600 MHz        18,106.100 kHz +/- 100 Hz
15m: 21.094600 MHz        21,096.100 kHz +/- 100 Hz
12m: 24.924600 MHz        24,926.100 kHz +/- 100 Hz
10m: 28.124600 MHz        28,126.100 kHz +/- 100 Hz 

WSPR Time Slots - Must start on an EVEN minute so some "Number of slots" can't be used. For example, 4 slots would mean TX every 15 minutes at 0,15,30,45. 7 would mean every 11 minutes
There are 2 distinct periods when you can transmit, 0,4,8,12,16 etc minutes and 2,6,10,14,18 etc minutes. I designated thes Odd and Even in the table.
Recomended TX 20% of the time which gives 3 slots in each TX period.
1 slot is different because minutes goes from 0 to 59 and we have to choose so that (minute modulus mins between TX) = 0. Any value above 30 will do.

		                                        	Period	1	2	3	4	5	 6  7	   8	9	  10	11	12	13	14	15	16	17	18	19	20	21	22	23	24	25	26	27	28	29	30 	Odd	Even
Num Slots	Mins TX 	%TX	      Mins between TX       	0	2	4	6	8	10	12	14	16	18	20	22	24	26	28	30	32	34	36	38	40	42	44	46	48	50	52	54	56	58		
     1       2	    3.33	     anything > 30                                                                                                     X                    1    0                                                                                                  																						                                            						
     2	     4	    6.67	          30	              X													                        X														                                 	1	   1
     3	     6	    10.00	          20	              X									           	X										                     X									                    	3	   0
     5	    10	    16.67         	12	              X					    X						             X					           	X						            X						              5	   0
     6	    12	    20.00	          10	              X					 X				            X					          X					           X					        X			                3    3 
     8	    16	    26.67	           8	              X				X				       X	 			       X	 			      X	 			       X	 			        X	 			         X	    	8    0
    10	    20	    33.33	           6	              X			X			  X			       X			     X		      X			      X			      X			      X			   X			     X    5	   5
    15	    30	    50.00	           4	              X		X   X	 	  X		     X	   	X		     X	 	   X		  X	    	X		   X	    	X		    X	 	     X		   X				15	 0


*/
/* Most boards have the built in LED connected to digital pin 13 and normally have a definition for LED_BUILTIN
However, I've recently come accros some boards that don't have LED_BUILTIN defined and the LED is connected to pin2. 
I've also come across ESP32 boards with no leds apart from the power indicator. If you have a board where LED_BUILTIN
 isn't defined, or there is no LED, the following code will define LED_BUILTIN = 2 and allow the program to compile and load.
This means that the LED will come on when WSPR is transmitting. If the led doesn't come on either there isn't an LED or
it's connected to another pin. ESP32's marked ESP32_Devkitc_V4 only have a power LED 
*/
#define DEBUG  // Comment this line to supress debugging to the serial port
bool fault = false;
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

/******************************[ Libraries ]***********************************************************************************/
#include "Wire.h"              // arduino
#include <si5351.h>            // arduino
#include <JTEncode.h>          // arduino
#include <Adafruit_GFX.h>      // arduino
#include <Adafruit_SSD1306.h>  // arduino
#include <TinyGPSPlus.h>       // arduino
#include <HardwareSerial.h>    // arduino

/******************************[ WSPR ]********************************************/
char callsign[7] = "******";  //  USER CALLSIGN
int randomChange = 0;         // 0 to 100.  a random value between -randomChange and +randomChange is applied to the TX frequency random(-100, 100)

// txPower is determined by si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA) + any amplification you add
int txPower = 10;         // your actual TX power in dBm.
int timeSlots = 6;       // number of slots to transmit in each hour. Allowed values 1,2,3,5,6,8,10 and 15 corresponding to 3.33% of the time 6.67%,10%,16.67%,20%,26.67%,33.33%,50%
#define TONE_SPACING 146  // ~1.46 Hz
#define WSPR_DELAY 683    // Delay value for WSPR
#define WSPR_CTC 10672    // CTC value for WSPR
#define SYMBOL_COUNT WSPR_SYMBOL_COUNT
JTEncode jtencode;                //create instance
uint8_t tx_buffer[SYMBOL_COUNT];  // create buffer to hold TX chars

/******************************[ si5351 ]*******************************************************************************************/
int32_t cal_factor = 8100;  //Calibration factor obtained from Calibration arduino program in Examples. You must calbrate first
Si5351 si5351(0x60);        // si5351 instance. I've put I2C address in because sometimes it didn't seem to respond without it
/******************************[ OLED Display ]**************************************************************************************/
int SCREEN_WIDTH = 128;  // OLED display width, in pixels
int SCREEN_HEIGHT = 64;  // OLED display height, in pixels
int charWidth = 6;       // 21 characters in 128 pixels
int lineSpace = 10;      // gives 6 lines of text with 21 characters a line
int timeLine = 0;        // time line
int dateLine = 1;        // date line
int latLongLine = 2;     // latitude and longitude line number
int locatorLine = 3;
int frequencyLine = 4;                                             // frequency line
int txOnLine = 5;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);  // create an instance of Adafruit_SSD1306 called display

/******************************[ GPS ]*****************************************************************************************/
#define RX_PIN 16  // ESP32 Pin connected to the TX of the GPS module
#define TX_PIN 17  // ESP32 Pin connected to the RX of the GPS module
const int GPSBaud = 9600;
HardwareSerial gpsPort(2);
TinyGPSPlus gps;
char locator[5] = "****";  // will be computed from gps latitude and longitude

/*****************************[ Other Global Variables]*******************************/
#define BAUDRATE 115200  // Arduino serial monitor
// WSPR TX frequency = dial + 1500Hz Uncomment only 1, the one you are using
//const unsigned long freq =  28126100UL;  // 10Mtrs
//const unsigned long freq =  24926100UL;  // 12Mtrs
//const unsigned long freq =  21096100UL;  // 15Mtrs
//const unsigned long freq = 18106100UL;  // 17Mtrs
const unsigned long freq = 14097100UL;  // 20Mtrs
//const unsigned long freq =  10140200UL;  // 30Mtrs
//const unsigned long freq =   7040100UL;  // 40Mtrs
//const unsigned long freq =   3568750UL;  // 80Mtrs
float freqMHz;         //  the frequency to show on the OLED e.g. 18.106100
unsigned long txFreq;  // the actual frequency being transmitted = freq +- random(randomChange)

/***************************** [ initialiseDisplay ] *****************************************
***                             Initialise the OLED                                        ***
**********************************************************************************************/
void initialiseDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address found in I2C_Scanner.ino = 0x3C
#ifdef DEBUG
    Serial.println("SSD1306 allocation failed");  // if the display isn't found you can't write an error message on it
    Serial.println(" Program will continue");
#endif
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.display();  // you need display.display() or you don't update the display
#ifdef DEBUG
    Serial.println("initialiseDisplay finished");
#endif
  }
}

/**************************** [ smartDelay] ************************************
***                    delay but keep reading GPS module                     ***
********************************************************************************/
static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (gpsPort.available())
      gps.encode(gpsPort.read());
  } while (millis() - start < ms);
}

/************************************[ initialiseGPS ] **************************************
***  initialise the GPS module and wait till a valid set of data is received              ***
*********************************************************************************************/

void initialiseGPS() {
  int attempts = 1;
  boolean validData = false;  // the isValid flag only indicates a valid sentence was received, it doesn't mean the data is valid

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Attempting GPS Start");
  display.display();
  gpsPort.begin(GPSBaud, SERIAL_8N1, RX_PIN, TX_PIN);
  gps.encode(gpsPort.read());
  validData = ((gps.date.day() > 0) && (gps.date.month() > 0) && (gps.date.year() > 0) && ((abs(gps.location.lat()) > 0.1)) && ((abs(gps.location.lng()) > 0.1)));  //apparently these all come back as zero if no fix but gps.date.isValid() is true
  while (!validData) {
    gps.encode(gpsPort.read());
    validData = ((gps.date.day() > 0) && (gps.date.month() > 0) && (gps.date.year() > 0) && ((abs(gps.location.lat()) > 0.1)) && ((abs(gps.location.lng()) > 0.1)));  //apparently these all come back as zero if no fix but gps.date.isValid() is true
#ifdef DEBUG                                                                                                                                                          // newlines on serial port
    if (attempts % 60 == 0) {
      Serial.println();
    } else {
      Serial.print("*");
    }
#endif

    if (attempts % 120 == 0) {  // new screen on OLED
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Attempting GPS Start");
    } else {
      display.print("*");
      display.display();
    }
    attempts++;
    smartDelay(200);
  }
  display.println();
  display.println("Got Fix");
#ifdef DEBUG
  Serial.println();
  Serial.println("Got Satellite Fix");
#endif
  display.display();
  delay(1000);  // or you won't see the starting messages
}

/**************************  [ initialiseSI5351 ] ***********************************
***                Initialse the si5351 square wave generator                     ***
*************************************************************************************/
void initialiseSI5351() {
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  si5351.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
  txFreq = freq + random(-randomChange, randomChange);
  freqMHz = txFreq / 1000000.0;                  // used to give the user a meaningful displayed frequency. If you divide with an integer, it only returns the quotient as an integer
  si5351.set_freq((txFreq * 100), SI5351_CLK0);  // Clock 0 used to tx WSPR
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  // If you change the drive strength to another value you will need to change the txPower value at line 76
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);  // 2,4,6,8mA  2 mA roughly corresponds to 3 dBm output,8 mA is approximately 10 dBm
  si5351.set_clock_pwr(SI5351_CLK0, 0);                  // Disable the clock initially
#ifdef DEBUG
  Serial.println("initialiseSI5351 finished");
#endif
}

/************************************ [ serialShowTime ] *************************************
***  Only used in debug. Show date and time in serial monitor                              ***
**********************************************************************************************/
void serialShowTime() {
  Serial.print("  ");
  serialPadZero(gps.date.day());
  Serial.print("/");
  serialPadZero(gps.date.month());
  Serial.print("/");
  serialPadZero(gps.date.year());
  Serial.print("  ");
  serialPadZero(gps.time.hour());
  Serial.print(":");
  serialPadZero(gps.time.minute());
  Serial.print(":");
  serialPadZero(gps.time.second());
}
/************************************ [ cursorAt] *************************************************
*** Sets the OLED cursor to the position of the character x chars along line on line lineNumber ***
***************************************************************************************************/
void cursorAt(int charPositionX, int lineNumber) {
  int displayY;                                            // y position to print the number
  displayY = (lineNumber)*lineSpace;                       // display top line = 0
  display.setCursor(charPositionX * charWidth, displayY);  // again, x starts at 0
}


/************************************ [ displayNumberAt ] ************************************
***  OLED display the number on lineNumber at x characters in                              ***
***  doing it this way should stop screen flicker as I will only be changing 1 piece  of   ***
***  data and not refreshing the whole screen Linenumber starts at 0, X  and Y start at 0  ***
**********************************************************************************************/
void displayNumberAt(int charPositionX, int lineNumber, int number) {
  cursorAt(charPositionX, lineNumber);
  display.setTextColor(WHITE, BLACK);  // this allows the spaces to act as blanks
  display.print("  ");                 // blank the old number with 2 spaces
  display.display();
  cursorAt(charPositionX, lineNumber);  // put the curser back 2 chars
  if (number < 10) {
    display.print("0");
    display.display();
  }
  display.print(number);
  display.display();
}
/********************************[ clearLine ]********************
***    OLED clear the line, overprint what's there with spaces ***
***    Linenumbers start at 0 for the top line                 ***
******************************************************************/
void clearLine(int lineNumber) {
  int i;
  cursorAt(0, lineNumber);
  display.setTextColor(WHITE, BLACK);  // this allows the spaces to act as blanks
  for (i = 0; i < 21; i++) {
    display.print(" ");
  }
  display.display();
}
/************************************ [ mainScreen ] *****************************************
***  This is the data I want to see all the time on the OLED                               ***
***  1st line   ***
**********************************************************************************************/
void mainScreen() {

  static bool firstRun = true;  // this will force mainScreen to update everything on first entry
  static int lastSecond = 99;
  static int lastMinute = 99;
  static int lastHour = 99;
  static int lastYear = 9999;
  static int lastMonth = 99;
  static int lastDay = 99;
  static float lastFreq = 0.0;
  if (firstRun) {
    display.clearDisplay();
    cursorAt(0, latLongLine);
    //  display.print("Lat ");
    display.print(gps.location.lat(), 5);
    if (gps.location.lat() > 0.0000001) {
      display.print("N ");
    } else {
      display.print("S ");
    }
    //  display.print("  Long ");
    display.print(gps.location.lng(), 5);
    if (gps.location.lng() > 0.0000001) {
      display.print("E");
    } else {
      display.print("W");
    }
    cursorAt(0, timeLine);
    //  display.print("UTC ");
    display.print("Time ");
    showTime();
    display.println(" UTC");
    getLocator();
    cursorAt(0, locatorLine);
    display.println(locator);
    firstRun = false;
    display.display();
  }

  if (!(lastDay == gps.date.day())) {  // the day has changed so update date
    lastDay = gps.date.day();
    clearLine(dateLine);
    cursorAt(0, dateLine);
    display.print(" Date ");
    showDate();
  }
  cursorAt(0, timeLine);
  if (!(lastHour == gps.time.hour())) {
    displayNumberAt(5, timeLine, gps.time.hour());
    display.print(":");
    display.display();
    lastHour = gps.time.hour();
  }
  if (!(lastMinute == gps.time.minute())) {
    displayNumberAt(8, timeLine, gps.time.minute());
    display.print(":");
    display.display();
    lastMinute = gps.time.minute();
  }
  if (!(lastSecond == gps.time.second())) {  // only display the main screen if the time has changed otherwise screen flashes
    displayNumberAt(11, timeLine, gps.time.second());
    display.display();
    lastSecond = gps.time.second();
    display.print(" UTC");
    display.display();
  }
  if (!(lastFreq == freqMHz)) {
    clearLine(frequencyLine);
    cursorAt(0, frequencyLine);
    display.print("Freq ");
    display.print(freqMHz, 6);
    display.print(" MHz");
    lastFreq = freqMHz;
    clearLine(locatorLine);
    cursorAt(0, locatorLine);
    display.print("Locator ");
    display.println(locator);
    display.display();
  }
  display.display();
#ifdef DEBUG
  Serial.println();
  Serial.print(freqMHz, 6);
  Serial.println(" MHz");
  serialShowTime();
  Serial.println();
  Serial.print("Callsign ");
  Serial.print(callsign);
  Serial.print("   ");
  Serial.print("Locator ");
  Serial.print(locator);
  Serial.print("  Power ");
  Serial.print(txPower);
  Serial.println("dBm");
#endif
}

/******************************** [ showDate ] ***************************************************
***          Display the Date on the OLED                                                      ***
**************************************************************************************************/
void showDate() {
  displayPadZero(gps.date.day());
  display.print("/");
  displayPadZero(gps.date.month());
  display.print("/");
  displayPadZero(gps.date.year());
  display.display();
}

/********************************** [ showTime ] *************************************************
***          Display the Time on the OLED                                                      ***
**************************************************************************************************/
void showTime() {
  displayPadZero(gps.time.hour());
  display.print(":");
  displayPadZero(gps.time.minute());
  display.print(":");
  displayPadZero(gps.time.second());
  display.display();
}

/************************** [ serialPadZero ] ******************************************
*** Print a "0" in front of a single digit number in the Serial Monitor Output       ***
****************************************************************************************/
void serialPadZero(int aNumber) {
  if (aNumber < 10) {
    Serial.print("0");
    Serial.print(aNumber);
  } else {
    Serial.print(aNumber);
  }
}

/**********************************  [ displayPadZero ]  ************************************
*** Print a "0" in front of a single digit number on the OLED display                     ***
*********************************************************************************************/
void displayPadZero(int aNumber) {
  if (aNumber < 10) {
    display.print("0");
    display.print(aNumber);
  } else {
    display.print(aNumber);
  }
  display.display();  // if you dont do display.display() it doesn't display. Very annoying
}

/********************* [ encode] *********************************
***                WSPR encode and transmit                    ***
*** When in this function the time on the OLED will not update ***
******************************************************************/
void encode() {
  uint8_t i;
#ifdef DEBUG
  Serial.println("*** Encode In ***");
#endif
  digitalWrite(LED_BUILTIN, HIGH);  // tell user we are now transmitting
  jtencode.wspr_encode(callsign, locator, txPower, tx_buffer);
  for (i = 0; i < SYMBOL_COUNT; i++) {
    si5351.set_freq((txFreq * 100) + (tx_buffer[i] * TONE_SPACING), SI5351_CLK0);
    delay(WSPR_DELAY);
  }
  // si5351.set_clock_pwr(SI5351_CLK0, 0);  // Finished TX so Turn off the output
  digitalWrite(LED_BUILTIN, LOW);  // tx off
#ifdef DEBUG
  Serial.println("*** Encode Out ***");
#endif
  mainScreen();
}

/***************************** [ txDelay ] ******************************************
***          txDelay() Return number of minutes between transmissions             ***
*************************************************************************************/
int txDelay(int numSlots) {
  int numMinutes = 10;  // 20% TX time, default value
  switch (numSlots) {
    case 1:             // 60 min cycle
      numMinutes = 38;  // any even number above 30 because usage is      gps.time.minute() % numMinutes == 0
      break;
    case 2:
      numMinutes = 30;
      break;
    case 3:
      numMinutes = 20;
      break;
    case 5:
      numMinutes = 12;
      break;
    case 6:
      numMinutes = 10;
      break;
    case 8:
      numMinutes = 8;
      break;
    case 10:
      numMinutes = 6;
      break;
    case 15:
      numMinutes = 4;
      break;
    default:
      numMinutes = 10;
      break;
  }
  return numMinutes;
}

/***************************** [ txOn ] ***********************************************
*** Neither switching TX on a couple of seconds early or having clock1 running at   ***
*** 150MHz when clock0 wasn't transmitting made any difference to my drift problem. ***
*** My drift problems were solved by using another si5351                           ***
**************************************************************************************/
void txOn() {
  txFreq = freq + random(-randomChange, randomChange);
  freqMHz = txFreq / 1000000.0;
  si5351.set_freq((txFreq * 100), SI5351_CLK0);
  si5351.set_clock_pwr(SI5351_CLK0, 1);  // switch on clock0
}

/************* [ txOff ] ***************
*** Switch clock0 off                ***
****************************************/
void txOff() {
  si5351.set_clock_pwr(SI5351_CLK0, 0);  // stop transmitting. Switch off clock 0
}
/***************************[getLocator]*****************************************************
*** Get the locator from the GPS latitude and longitude                                   ***
*********************************************************************************************/
void getLocator() {
  float latitude{ gps.location.lat() + 90.0 };
  float longitude{ gps.location.lng() + 180.0 };

  locator[0] = 'A' + (longitude / 20);
  locator[1] = 'A' + (latitude / 10);

  locator[2] = '0' + (int(longitude / 2) % 10);
  locator[3] = '0' + (int(latitude) % 10);

  locator[4] = '\0';
}
void errorDetected() {
  while (1)
    ;
}
/***********************[setup]********************************************
***   The setup() function runs only once at the start of the program   ***
***   When it gets to the end of the loop it goes back to the beginning ***
***************************************************************************/
void setup() {
  int i = 0;
#ifdef DEBUG
  Serial.begin(BAUDRATE);
  delay(2000);  // give port time to open
#endif
  pinMode(LED_BUILTIN, OUTPUT);  // used as indicator that we are transmitting.
  initialiseDisplay();
  display.setCursor(0, 10);
  display.println("OLED OK");
  display.display();
  initialiseGPS();
  display.println("    GPS  OK - Got Fix");
  initialiseSI5351();
  display.println("    si5351 initialised");
  display.display();
  delay(1000);                      // stop for a second to let user read screen
  digitalWrite(LED_BUILTIN, HIGH);  // flash the LED if there is one
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  display.display();
  randomSeed(analogRead(0));  // get an arbitrary starting seed
  if (callsign[0] == '*') {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("## Callsign Error ##");
    display.print("##   ");
    display.print(callsign);
    display.println("       ##");
    Serial.println("###########################################");
    Serial.print("#### Callsign ERROR  ");
    Serial.print(callsign);
    Serial.println("            ####");
    Serial.println("###########################################");
    fault = true;
  }

  if ((randomChange > 100) || (randomChange < 0)) {
    if (!fault) {  // the callsign was ok
      display.clearDisplay();
      display.setCursor(0, 10);
    }
    display.print("randomChange = ");
    display.println(randomChange);
    display.println("Should be 0 to 100");
    display.display();
    Serial.println("###########################################");
    Serial.print("#### ");
    Serial.print("randomChange = ");
    Serial.print(randomChange);
    Serial.println("                ####");
    Serial.println("####    It must be >= 0 and <= 100     ####");
    Serial.println("###########################################");
    Serial.println("*******    Program Halted    *****************");
    fault = true;
  }
  if (fault) {
    display.println("Program Halted");
    for (;;) {
      // stay here for ever if there's an error on startup
    }
  }
  mainScreen();
}
/***************************[Loop]*****************************************
***   The loop() function runs continuously after setup.                ***
***   When it gets to the end of the loop it goes back to the beginning ***
***************************************************************************/
void loop() {
  while (gpsPort.available() > 0) {
    if (gps.encode(gpsPort.read())) {
    }
  }
  mainScreen();
  if ((gps.time.minute() % txDelay(timeSlots) == 0) && (gps.time.second() == 0)) {  //start encoding at start of even minute
    digitalWrite(LED_BUILTIN, HIGH);
    txOn();
#ifdef DEBUG
    Serial.println("************************************************* Loop TX ON *******");
#endif
    clearLine(txOnLine);
    cursorAt(0, txOnLine);
    display.print("Loop TX ON");
    display.display();
    encode();  // transmit the codes
    txOff();
    clearLine(txOnLine);
    smartDelay(2000);  // give the gps time to get to the next second before going round again
#ifdef DEBUG
    Serial.println("************************************************* Loop TX OFF *******");
#endif
    digitalWrite(LED_BUILTIN, LOW);
  }
}
//*************************************[ END OF PROGRAM ]********************************************************
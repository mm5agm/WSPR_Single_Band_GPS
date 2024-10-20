# WSPR_Single_Band_GPS

If you are new to the Arduino and ESP32 please read the readme.md in my WSPR repository for a comprehensive setup guide.
Instead of loading NTP_Basis.ino you should load GPS_Basic.ino, then GPS_Time_Location_OLED.ino, then WSPR_Single_Band_GPS.ino

My code uses Serial2, marked RX2 and TX2 on some boards to connect the GPS receiver to the ESP32.

![ESP32_Single_Band_WSPR_GPS_bb](https://github.com/mm5agm/WSPR_Single_Band_GPS/assets/26571503/f35b96cd-5774-45d3-bf09-22ae6d47d925)

The ESP32 comes in many varieties. Some give 5v at the Vin pin some a bit less than 3.3V

ESP32 marked "ESP32 DEVKITV1" Gives the 5V I required for my GPS module but another ESP32 marked "ESP32_DEVKIkc_V4" doesn't. The ESP32_DEVKIkc_V4 also lacks an LED apart from the power one. I use the "ESP32 DEVKITV1" and in the Arduino IDE I select "DOIT ESP32 DEVKIT V1" as the board.

This is a GPS version of my single band wspr beacon. No need for internet or NTP servers.

I've used the TinyGPSPlus library. In TinyGPSPlus, isValid, doesn't mean that the data is valid, it means that the sentence received was valid. In the case of the date, a valid sentence could contain month, day, and year all equal to 0, and the isValid flag will be true.


 1. In the Arduino IDE go to “File/Preferences” and fill in the “Additional boards manager URLs” with https://espressif.github.io/arduino-esp32/package_esp32_index.json

2. Get time from GPS - Load and compile GPS_Basic.ino in Arduino IDE. Connect ESP32 TX to GPS RX and ESP32 RX to GPS TX

3. Add OLED display, Scan I2C to get OLED address, use I2C_Scanner.ino - I didn't need to use the OLED address

4. Display information on OLED - GPS_Time_RTC_OLED.ino

5. Add si5351 square wave generator - calibrate using Examples/Etherkit Si5351/si5351_calibration.ino in the Arduino IDE. I changed target_freq on line 32 from 1000000000ULL to 10140200000ULL and used WSJT-X in WSPR mode on 30mtrs to see where my signal was.

6. Add low pass filter for the band you are using.

7. Open WSPR_Single_Band_GPS.ino in the Arduino IDE.

8. Change the callsign (about line 89) and, if you don't want 20mtrs, comment out the 20mtrs (around line 133) and uncomment the frequency you want.
   
10. Compile and Load WSPR_Single_Band_GPS.ino into your ESP32 after changing the callsign to yours.

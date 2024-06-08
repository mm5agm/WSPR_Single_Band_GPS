

![ESP32_Single_Band_WSPR_GPS_bb](https://github.com/mm5agm/WSPR_Single_Band_GPS/assets/26571503/f35b96cd-5774-45d3-bf09-22ae6d47d925)


# WSPR_Single_Band_GPS
The ESP32 comes in many varieties. Some give 5v at the Vin pin some a bit less than 3.3
ESP32 marked "ESP32 DEVKITV1" Gives the 5V I required for my GPS module but another ESP32 marked "ESP32_DEVKIkc_V4" doesn't. The ESP32_DEVKIkc_V4 also lacks an LED apart from the power one.

This is a GPS version of my single band wspr beacon. No need for internet or NTP servers.

I've still to add the final program - possibly later this week
 A Single Band WSPR beacon using GPS to sync time
 
 1. In the Arduino IDE go to “File/Preferences” and fill in the “Additional boards manager URLs” with https://espressif.github.io/arduino-esp32/package_esp32_index.json

2. Get time from GPS - GPS_Basic.ino Connect ESP32 TX to GPS RX and ESP32 RX to GPS TX

3. Add OLED display, Scan I2C to get OLED address - I didn't need to use the OLED address

4. Display information on OLED - GPS_Time_RTC_OLED.ino

5. Add si5351 square wave generator - calibrate using Examples/Etherkit Si5351/si5351_calibration.ino in the Arduino IDE. I changed target_freq on line 32 from 1000000000ULL to 10140200000ULL and used WSJT-X in WSPR mode on 30mtrs to see where my signal was.

6. Add low pass filter for the band you are using.

I'll add the final program later this week

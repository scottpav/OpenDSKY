# OpenDSKY
Modified code from https://opendsky.backerkit.com/
Departures from S&T GeoTronics Code
1. Using Adafruit GPS with Software Serial on pins 8, 9.
2. No line level
3. No reed relay


Available functions as of 06/22/2018:

1. V16 N17 - Display XYZ Values from IMU
2. V16 N36 - Read Time From RTC
3. V16 N43 - Display GPS Position & Altitude
4. V16 N87 - Display IMU XYZ values with simulated 1202 Alarm
5. V21 N36 - Set the Time on RTC Module
6. V16 N46 - Display GPS Velocity & Altitude
7. V16 N33 - User Configurable Countdown Timer

# Antenna-Tester
LoRa Antenna Tester

Forked from the guy with the Swiss Accent.

Sender sketch is made for a TTGO V1 ESP32 board with a Oled and a LoRa chip. You have to change the pins if you want to use it on another platform.
Pressing 'prog' button starts a test to calculate average RSSI. Holding 'prog' button > 3 seconds and then releasing "zero's" device to the last average value.

Receiver sketch is made for a feather M0 board connected to a shield with a RFM95 and GPS module. Again, should be easy to change to make work for the TTGO.

.. _hikari-sword-app:

Hikari Sword Application
########################

Overview
********

Lighting controller for hikari sword replica.
Compatible with arduino nano 33 ble board.

Use "west build -b arduino_nano_33_ble" to build application.
Use "west flash --bossac=$HOME/.arduino15/packages/arduino/tools/bossac/1.9.1-arduino2/bossac" to flash application to the board.

Use "minicom -b 115200 -D /dev/ttyACM0" to connect to serial over USB for debugging.





TODO:
1. fix/finish the wave mode. Make code with changes buildable.
2. fix USB serial connection issues. Maybe something with a UDEV rule that recognizes
   the device and attaches it as a uart device as ttyACM0.

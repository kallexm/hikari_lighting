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

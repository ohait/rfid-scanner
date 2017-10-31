# Collection of resources for the arduino project

This folder contains arduino source files, fritzing parts and 3d models.

It is completely a work in progress, so nothing is probably usable as it is, but feel free to contact me and see if I can help you.

## RFID Scanner

The main aim of this project is to design and 3d-printable rfid scanner which can be used by libraries to do inventories and check-in

It connects to a server via WiFi, and depends on it for the logic. IOW the device is pretty dumb, it scans and tells the server, and the servers tells back what to display. (cfr. [documentation](../doc/scanner_protocol.md)

The server could (and probably should) integrate with your ILS (Integrated Library System).

## Power

the device consume ~210mA when active, and ~10mA when standby

the 1200mAh LiPo battery should provide up to 6 hours of continuous scanning, and it will recharge in 10/12 hours

## installation

follow instruction here to install the board 

https://learn.adafruit.com/adafruit-feather-huzzah-esp8266/using-arduino-ide


then install adafruit GFX for the oled display

arduino ide => manage libraries => adafruit GFX


also, you will need specific library for SSD1305 and the RFID module library:

in `arduino/libraries/`

`git clone git@github.com:adafruit/Adafruit_SSD1305_Library.git`

some patches are required for the 128x32 version <<TODO>>

`git clone git@github.com:ohait/jmy6xx.git`



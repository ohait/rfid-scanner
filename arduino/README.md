# Collection of resources for the arduino project

This folder contains arduino source files, fritzing parts and 3d models.

It is completely a work in progress, so nothing is probably usable as it is, but feel free to contact me and see if I can help you.

## RFID Scanner

The main aim of this project is to design and 3d-printable rfid scanner which can be used by libraries to do inventories and check-in

It connects to a server via WiFi, and depends on it for the logic. IOW the device is pretty dumb, it scans and tells the server, and the servers tells back what to display. (cfr. [documentation](../doc/scanner_protocol.md)

The server could (and probably should) integrate with your ILS (Integrated Library System).

## Shelves

The scanner will recognize rfid tags which contains specific `SHELF#<data>`, and use the give data as a mean for the server to know where the scanner is currently scanning.

The server might decide that one of the scanned items is misplaced, and send a message back to the scanner to play a sound and update the display with relevant information on which item specifically need to be picked up.

A momentary button can be used to either reset the shelf location (quick press) or perform a "check in" (hold, and scan again).

Check-In scans are implementation specific, but most likely it will inform the server, and the integrated ILS, that the user actually picked up the item.

## Other integration

While the whole project is designed for libraries, I can't see why this couldn't be used differently.

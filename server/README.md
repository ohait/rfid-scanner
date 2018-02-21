# RFID Scanner Server
The server is responsible to store the scanned items from different scanners, and reply back to the scanners if some of the items need to be picked up

## RFID data parse
the server parse the data in the RFID tag.
Currently it recognize DS-24 (more details [here](http://biblstandard.dk/rfid/dk/RFID_Data_Model_for_Libraries_February_2009.pdf))
The data will have a barcode, which can be used to integrate with other ILS

## Integration with ILS
Thre is a WIP integration with [Koha](http://koha.org) which returns metadata for given 

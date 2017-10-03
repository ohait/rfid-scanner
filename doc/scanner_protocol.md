# RFID communication

The scanner can talk with the server using different protocols, although right now only http is implemented.

## http
```
POST /myURL/8266_128x32 HTTP/1.1
Host: rfidscanner.mydomain.com
Length: <REQLEN>

<REQ>
```
which the server replies with
```
HTTP/1.1 200 OK
Content-Length: <RESLEN>

<RES>
```


# RFID Scanner protocol
```
HEADER:
Bytes  Desc
2      0x4242 (a protocol number)
6      MAC ADDRESS of the rfid device
32     shelf
[one or more RECORDs]

RECORD:
8      RFID tag id
1      flags (bitmask: 1 => checkin)
1      data length (N)
N      data
```
After the scanner found 1 or more RFID tags, it will send a request to the server with a header and one or more records.

anything scanned will be sent to the server.

each tags following a shelf tag belongs to that shelf, and not the initial shelf value.

for a shelf, the data will contains `SHELF#shelf.name.here`

# RESPONSE

The server might respond with:

## READ
```
READ\n
$rfid (8 bytes)\n
```
The scanner should re-scan (Reset To Ready) the tag, and read the tag data again.

## WRITE
```
WRT\n
length($rfid+$data)\n
$rfid+$data
```
The scanner need to overwrite the tag with the given RFID id, with $data. (the $rfid is 8 bytes and is the rfid id that need to be written, while $data is the data to be written)

The scanner should then read the tag again and send the content back to the server as a confirmation.

## IMG
```
IMG\n
priority\n
base64 chars\n
[...]
```
Send a bitmap to the client, with the given `priority` (Higher priority wins).

The data and number of base64 strings can be encoded differently for different devices.

## TONE
```
TONE\n
GIVEN\n
```
The device should play the `GIVEN` sound.

right now, those sounds are supported:

|tone|description|
|----|-----------|
|PICK|an item needs to be picked up|
|KO|some generic errors|


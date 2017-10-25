### Architecture

```
+--------+           
|Scanner +--+          +-------------+
+--------+  |          | Web Browser |
            v          +--+----------+
         +--------+       |
         |        | <-----+
         | Server |
     +---+        |
     |   +-+-+-+-++
     | SQL | | | |     +--------+
     |     | | | +---> | SIP    |
     +-----+ | |       +--------+
             | |     +---------+
             | +---> | Koha    |
             |       +---------+
             |    +----------+
             +--> | OtherILS |
                  +----------+
```

# Scanner

The scanner is the 3d printed Arduino device which scan and read rfid tags

it connects to the Server via WiFi

# Server

The Server is The central point that gather information and send instructions back to the scanner

It listen to connections from the Scanner, From web clients and it connects to ILS systems via SIP or other apis

It also connect to a SQL database

# SIP, Koha, OtherILS

Those are the systems currently in use to handle the circulation of your items


## Typical scenario

```
                  +---------+       +---------+           +-----+
                  | Scanner |       | Server  |           | ILS |
                  +---------+       +---------+           +-----+
                       |                 |                   |
      +--------------\ |                 |                   |
      | Item Scanned |-|                 |                   |
      +--------------+ |                 |                   |
                       |                 |                   |
                       | rfid data       |                   |
                       |---------------->|                   |
                       |                 |                   |
                       |                 | parse data        |
                       |                 |-----------        |
                       |                 |          |        |
                       |                 |<----------        |
                       |                 |                   |
                       |                 | info(barcode)     |
                       |                 |------------------>|  /-----------------+
                       |                 |                   |--| There is a hold |
                       |                 |    Metadata,Holds |  +-----------------+
                       |                 |<------------------|
                       |                 |                   |
                       |      info,tone  |                   |
                       |<----------------|                   |
+--------------------\ |                 |                   |
| Beep! Pick that up |-|                 |                   |
+--------------------+ |                 |                   |
                       |                 |                   |
```

In this scenario, the scanner is in inventory mode, it is scanning tags and find one.

the data is sent as-is to the server.

The server parse the data, recognize the Danish S24 standard, and fetch library + barcode

it connects to the ILS asking for information about the item by barcode

The ILS returns title, author, callnumber, and any other useful identifier.
It also add information about the status of the item, and if there are holds waiting for it.

The Server use the current location of the scanner, and the status fetched from ILS to determine the item needs to be moved elsewhere.

A message is then sent back to the client with the details of the item and a type of tone to be played.


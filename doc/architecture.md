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


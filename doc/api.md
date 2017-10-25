#### API

The server exposes APIs for querying the status of shelves, items or devices.

### Common practices

* Timestamps are not in ISO8601 but instead in seconds from 1st Jan 1970 UTC, with decimals when available.
* All responses will be a json object, with execution timestamp as `requeste_at`

## `/json/shelf/`
```
GET /json/shelf/flam.pub
{
   "parent" : "flam",
   "path" : "/json/shelf/flam.pub",
   "requested_at" : 1508931322.6837,
   "results" : [
      {
         "count" : 0,
         "inherited" : 5515,
         "name" : "flam.pub.2u"
      },
      {
         "count" : 0,
         "inherited" : 1315,
         "name" : "flam.pub.3"
      }
   ],
   "shelf" : "flam.pub"
}
```

given a shelf (could be empty), returns all the shelves which belong to that shelf.

`count` is the number of items on that shelf, while `inherited` are the items in shelves which are subshelves of the shelf.

## `/json/bibnum/`
```
GET /json/bibnum/1591338
{
   "path" : "/json/bibnum/1591338",
   "requested_at" : 1508931576.67009,
   "results" : {
      "03011591338003" : null,
      "03011591338006" : "flam.pub.3.c1",
      "03011591338007" : null,
      "03011591338012" : null,
      "03011591338013" : null,
      "03011591338014" : null,
      "03011591338017" : null
   },
   "type" : "bibnum"
}
```
returns all the known barcodes that belong to the given biblionumber, and the shelf they belong to

TODO! this API might change in the future to accomodate permanent/temporary locations

## `/json/dev/`
```
{
   "dev" : "5c:cf:7f:f0:ae:34",
   "path" : "/json/dev/5c:cf:7f:f0:ae:34",
   "requested_at" : 1508932281.21636,
   "results" : [
      {
         "action" : "PICK (pickup)",
         "at" : 1508774441,
         "data" : "\u0011\u0001\u000130119116637816\u0000\u0000cNO02030000\u0000\u0000\u0000\u0000\u0000",
         "rfid" : "e0:04:01:50:5a:89:94:b6",
         "shelf" : "flam.pub.3.d4",
         "type" : null
      },
      {
         "action" : null,
         "at" : 1508774441,
         "data" : "\u0011\u0001\u00011003011681558012\f×NO02030000\u0000\u0000\u0000\u0000\u0000",
         "rfid" : "e0:04:01:50:4e:90:8a:dd",
         "shelf" : "flam.pub.3.d4",
         "type" : null
      },
[...]
   ]
}
```
returns the last entries in the log for the given device

data can be different for different device, and will be purged when needed

## `/json/tag/`


```
GET /json/tag/e0:04:01:50:4e:94:06:ec
{
   "entry" : {
      "barcode" : "03011591338006",
      "history" : [
         {
            "action" : null,
            "at" : 1508783227.79596,
            "data" : "{\"sender\":\"tagvision_sip_proxy\",\"branch\":\"flam\",\"client_IP\":\"10.172.15.7\",\"barcode\":\"03011591338006\",\"sip_message_type\":\"MsgReqCheckout\"}",
            "dev" : "flam.tagvision_sip_proxy.10.172.15.7",
            "shelf" : "flam.auto.out",
            "type" : "MsgReqCheckout"
         }, 
         {
            "action" : null,
            "at" : 1508771570,
            "data" : "\u0011\u0001\u00011003011591338006/\u0003NO02030000\u0000\u0000\u0000\u0000\u0000",
            "dev" : "5c:cf:7f:f0:ae:34",
            "shelf" : "flam.pub.3.c1",
            "type" : null
         }, 
         {
            "action" : null,
            "at" : 1503658244.07074,
            "data" : null,
            "dev" : "flam.tagvision_sip_proxy.10.172.15.7",
            "rfid" : "e0:04:01:50:4e:94:06:ec",
            "shelf" : null,
            "type" : "MsgReqCheckout"
         },
         {
            "action" : null,
            "at" : 1503387770.81755,
            "data" : null,
            "dev" : "flam.rfidhub.10.172.15.102",
            "rfid" : "e0:04:01:50:4e:94:06:ec",
            "shelf" : null,
            "type" : "MsgReqCheckin"
         }
      ],
      "last_at" : 1508783227.7958,
      "last_dev" : "flam.tagvision_sip_proxy.10.172.15.7",
      "rfid" : "e0:04:01:50:4e:94:06:ec",
      "shelf" : "flam.pub.3.c1",
      "shelf_at" : 1508771572,
      "shelf_dev" : "5c:cf:7f:f0:ae:34",
      "temp" : "flam.auto.out",
      "temp_at" : 1508783227.7958
   },
   "path" : "/json/tag/e0:04:01:50:4e:94:06:ec",
   "requested_at" : 1508932506.04825,
   "type" : "tag_history"
}
```

returns the last entries of the log for a give rfid tag

## `/json/<barcode>` 
```
GET /json/
{
   "after" : [],
   "before" : [],
   "entry" : {
      "author" : "Henriksen, Levi",
      "barcode" : "03011591338006",
      "bibnum" : 1591338,
      "cn" : "dc CD Hen",
      "last_at" : 1508783227.7958,
      "last_dev" : "flam.tagvision_sip_proxy.10.172.15.7",
      "pubid" : "pe9e41ee6302d24a3f8c85d8671804333",
      "rfid" : "e0:04:01:50:4e:94:06:ec",
      "shelf" : "flam.pub.3.c1",
      "shelf_at" : 1508771572,
      "shelf_dev" : "5c:cf:7f:f0:ae:34",
      "temp" : "flam.auto.out",
      "temp_at" : 1508783227.7958,
      "thumb" : "https://static.deichman.no/thumb/pe9e41ee6302d24a3f8c85d8671804333.jpg",
      "title" : "Harpesang"
   }, 
   "history" : [
      {
         "action" : null,
         "at" : 1508783227.79596,
         "dev" : "flam.tagvision_sip_proxy.10.172.15.7",
         "rfid" : "e0:04:01:50:4e:94:06:ec",
         "type" : "MsgReqCheckout"
      }, 
      {
         "action" : null,
         "at" : 1508771572,
         "dev" : "5c:cf:7f:f0:ae:34",
         "rfid" : "e0:04:01:50:4e:94:06:ec",
         "type" : null
      }, 

      {
         "action" : null,
         "at" : 1501668630.12409,
         "dev" : "flam.tagvision_sip_proxy.10.172.15.7",
         "rfid" : "e0:04:01:50:4e:94:06:ec",
         "type" : "MsgReqCheckout"
      }
   ],
   "path" : "/json/03011591338006",
   "requested_at" : 1508932697.15131,
   "type" : "nearby"
}
```

returns all the details for a given barcode

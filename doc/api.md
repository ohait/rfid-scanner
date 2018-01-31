# API

The server exposes APIs for querying the status of shelves, items or devices.

### `/json/shelves/`
```
GET /json/shelves/
{
   at: {
      epoch: 1517398770.60357,
      iso8601: "2018-01-31T11:39:30"
   },
   results: [
      {
         loc: "x.y.z",
         tags: "123"
      },
      {
         loc: "hutl.stack.foo",
         tags: "2"
      },
      ...
   ]
}
```

### `/json/shelf/x.y.z`
```
GET /json/shelf/flam.pub
{
   at: {
      epoch: 1517398904.08518,
      iso8601: "2018-01-31T11:41:44"
   },
   results: [
      {
         data: {
            author: "Ottesen-Jensen, Elise",
            biblionumber: "34815",
            callnumber: ",612.6,Ott,",
            copynumber: "3",
            title: "Når barnet spør"
         },
         item_id: "03010034815003",
         item_supplier: "NO:02030000",
         last_at: {
            epoch: "1517320059.55546",
            iso8601: "2018-01-30T13:47:39"
         },
         product_id: "34815",
         tags: [
            {
               data: {
                  base64: "EQICMDMwMTAwMzQ4MTUwMDMAADgCTk8wMjAzMDAwMAAAAAAA"
               },
               permanent: {
                  at: {
                     epoch: "1517320059.55546",
                     iso8601: "2018-01-30T13:47:39"
                  },
                  dev: "5c:cf:7f:3a:37:45",
                  loc: "hutl.stack.foo"
               },
               rfid: "e0:04:01:50:5a:89:bd:e3",
               temporary: {
                  at: {
                     epoch: "1517321892.8167",
                     iso8601: "2018-01-30T14:18:12"
                  },
                  dev: "5c:cf:7f:3a:37:45",
                  loc: ""
               }
            },
            // more entries if multiple tags for an item
         ]
      },
      [...]
   ]
}
```

### `/json/product/1234`
```
GET /json/bibnum/1591338
```

### `/json/item/NO:1234/0123456789`
```
GET /api/item/NO:02030000/03010034815003
{
   at: {
      epoch: 1517399164.71528,
      iso8601: "2018-01-31T11:46:04"
   },
   item_id: "03010034815003",
   item_supplier: "NO:02030000",
   response: {
      history: [
         {
            action: null,
            at: {
               epoch: "1517321892.89494",
               iso8601: "2018-01-30T14:18:12"
            },
            dev: "5c:cf:7f:3a:37:45",
            location: "",
            type: null
         },
         {
            action: null,
            at: {
               epoch: "1517321892.89213",
               iso8601: "2018-01-30T14:18:12"
            },
            dev: "5c:cf:7f:3a:37:45",
            location: "",
            type: null
         },
         [...]
      ],
      item_id: "03010034815003",
      item_supplier: "NO:02030000",
      last_seen: {
         epoch: "1517321892.89494",
         iso8601: "2018-01-30T14:18:12"
      },
      meta: {
         author: "Ottesen-Jensen, Elise",
         biblionumber: "34815",
         callnumber: ",612.6,Ott,",
         copynumber: "3",
         title: "Når barnet spør"
      },
      product_id: "34815",
      tags: [
         {
            data: {
               base64: "EQICMDMwMTAwMzQ4MTUwMDMAADgCTk8wMjAzMDAwMAAAAAAA"
            },
            permanent: {
               at: {
                  epoch: "1517320059.55546",
                  iso8601: "2018-01-30T13:47:39"
               },
               dev: "5c:cf:7f:3a:37:45",
               loc: "hutl.stack.foo"
            },
            rfid: "e0:04:01:50:5a:89:bd:e3",
            temporary: {
               at: {
                  epoch: "1517321892.8167",
                  iso8601: "2018-01-30T14:18:12"
               },
               dev: "5c:cf:7f:3a:37:45",
               loc: ""
            }
         },
         [...]
      ]
   }
}
```
returns the last entries in the log for the given device

data can be different for different device, and will be purged when needed

### `/json/tag/e0:01:23:45:67:89:ab:cd`

### `/json/dev/e0:00:11:22:33:44`


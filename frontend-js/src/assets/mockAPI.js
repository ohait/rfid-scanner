// GET /json/shelves/
const mockShelves = {
   "at" : {
      "epoch" : 1519653867.39581,
      "in" : "now",
      "iso8601" : "2018-02-26T14:04:27"
   },
   "results" : [
      {
         "loc" : "hutl.stack.foo",
         "tags" : "2"
      },
      {
         "loc" : "",
         "tags" : "12"
      },
      {
         "loc" : "fbje.auto",
         "tags" : "193"
      },
      {
         "loc" : "fbje.auto.in",
         "tags" : "306"
      },
      {
         "loc" : "fbje.auto.out",
         "tags" : "477"
      },
      {
         "loc" : "fbje.staff",
         "tags" : "12"
      },
      {
         "loc" : "fbje.staff.in",
         "tags" : "411"
      },
      {
         "loc" : "fbje.staff.out",
         "tags" : "38"
      },
      {
         "loc" : "fbjo.auto",
         "tags" : "15"
      },
      {
         "loc" : "fbjo.auto.in",
         "tags" : "92"
      },
      {
         "loc" : "fbjo.auto.out",
         "tags" : "86"
      },
      {
         "loc" : "fbjo.staff",
         "tags" : "14"
      },
      {
         "loc" : "fbjo.staff.in",
         "tags" : "362"
      },
      {
         "loc" : "fbjo.staff.out",
         "tags" : "21"
      },
      {
         "loc" : "fbol.auto",
         "tags" : "241"
      },
      {
         "loc" : "fbol.auto.in",
         "tags" : "272"
      },
      {
         "loc" : "fbol.auto.out",
         "tags" : "432"
      },
      {
         "loc" : "fbol.staff",
         "tags" : "6"
      },
      {
         "loc" : "fbol.staff.in",
         "tags" : "536"
      },
      {
         "loc" : "fbol.staff.out",
         "tags" : "41"
      },
      {
         "loc" : "ffur.auto",
         "tags" : "219"
      },
      {
         "loc" : "ffur.auto.in",
         "tags" : "391"
      },
      {
         "loc" : "ffur.auto.out",
         "tags" : "595"
      },
      {
         "loc" : "ffur.staff",
         "tags" : "16"
      },
      {
         "loc" : "ffur.staff.in",
         "tags" : "512"
      },
      {
         "loc" : "ffur.staff.out",
         "tags" : "42"
      },
      {
         "loc" : "fgam.auto",
         "tags" : "338"
      },
      {
         "loc" : "fgam.auto.in",
         "tags" : "338"
      },
      {
         "loc" : "fgam.auto.out",
         "tags" : "792"
      },
      {
         "loc" : "fgam.staff",
         "tags" : "34"
      },
      {
         "loc" : "fgam.staff.in",
         "tags" : "638"
      },
      {
         "loc" : "fgam.staff.out",
         "tags" : "24"
      },
      {
         "loc" : "fgry.auto",
         "tags" : "200"
      },
      {
         "loc" : "fgry.auto.in",
         "tags" : "440"
      },
      {
         "loc" : "fgry.auto.out",
         "tags" : "609"
      },
      {
         "loc" : "fgry.staff",
         "tags" : "6"
      },
      {
         "loc" : "fgry.staff.in",
         "tags" : "465"
      },
      {
         "loc" : "fgry.staff.out",
         "tags" : "42"
      },
      {
         "loc" : "fhol.auto",
         "tags" : "114"
      },
      {
         "loc" : "fhol.auto.in",
         "tags" : "336"
      },
      {
         "loc" : "fhol.auto.out",
         "tags" : "455"
      },
      {
         "loc" : "fhol.staff",
         "tags" : "44"
      },
      {
         "loc" : "fhol.staff.in",
         "tags" : "618"
      },
      {
         "loc" : "fhol.staff.out",
         "tags" : "27"
      },
      {
         "loc" : "flam.auto",
         "tags" : "471"
      },
      {
         "loc" : "flam.auto.in",
         "tags" : "852"
      },
      {
         "loc" : "flam.auto.out",
         "tags" : "1446"
      },
      {
         "loc" : "flam.staff",
         "tags" : "5"
      },
      {
         "loc" : "flam.staff.in",
         "tags" : "825"
      },
      {
         "loc" : "flam.staff.out",
         "tags" : "82"
      },
      {
         "loc" : "fmaj.auto",
         "tags" : "1278"
      },
      {
         "loc" : "fmaj.auto.in",
         "tags" : "907"
      },
      {
         "loc" : "fmaj.auto.out",
         "tags" : "1775"
      },
      {
         "loc" : "fmaj.staff",
         "tags" : "9"
      },
      {
         "loc" : "fmaj.staff.in",
         "tags" : "1267"
      },
      {
         "loc" : "fmaj.staff.out",
         "tags" : "64"
      },
      {
         "loc" : "fnor.auto",
         "tags" : "85"
      },
      {
         "loc" : "fnor.auto.in",
         "tags" : "194"
      },
      {
         "loc" : "fnor.auto.out",
         "tags" : "179"
      },
      {
         "loc" : "fnor.staff",
         "tags" : "1"
      },
      {
         "loc" : "fnor.staff.in",
         "tags" : "286"
      },
      {
         "loc" : "fnor.staff.out",
         "tags" : "11"
      },
      {
         "loc" : "fnyd.auto",
         "tags" : "116"
      },
      {
         "loc" : "fnyd.auto.in",
         "tags" : "116"
      },
      {
         "loc" : "fnyd.auto.out",
         "tags" : "261"
      },
      {
         "loc" : "fnyd.staff",
         "tags" : "12"
      },
      {
         "loc" : "fnyd.staff.in",
         "tags" : "400"
      },
      {
         "loc" : "fnyd.staff.out",
         "tags" : "44"
      },
      {
         "loc" : "fopp.auto",
         "tags" : "156"
      },
      {
         "loc" : "fopp.auto.in",
         "tags" : "169"
      },
      {
         "loc" : "fopp.auto.out",
         "tags" : "317"
      },
      {
         "loc" : "fopp.staff",
         "tags" : "5"
      },
      {
         "loc" : "fopp.staff.in",
         "tags" : "401"
      },
      {
         "loc" : "fopp.staff.out",
         "tags" : "20"
      },
      {
         "loc" : "frmm.auto",
         "tags" : "224"
      },
      {
         "loc" : "frmm.auto.in",
         "tags" : "191"
      },
      {
         "loc" : "frmm.auto.out",
         "tags" : "211"
      },
      {
         "loc" : "frmm.staff.in",
         "tags" : "285"
      },
      {
         "loc" : "frmm.staff.out",
         "tags" : "44"
      },
      {
         "loc" : "froa.auto",
         "tags" : "248"
      },
      {
         "loc" : "froa.auto.in",
         "tags" : "427"
      },
      {
         "loc" : "froa.auto.out",
         "tags" : "786"
      },
      {
         "loc" : "froa.staff",
         "tags" : "19"
      },
      {
         "loc" : "froa.staff.in",
         "tags" : "872"
      },
      {
         "loc" : "froa.staff.out",
         "tags" : "12"
      },
      {
         "loc" : "from.auto",
         "tags" : "33"
      },
      {
         "loc" : "from.auto.in",
         "tags" : "77"
      },
      {
         "loc" : "from.auto.out",
         "tags" : "106"
      },
      {
         "loc" : "from.staff",
         "tags" : "1"
      },
      {
         "loc" : "from.staff.in",
         "tags" : "211"
      },
      {
         "loc" : "from.staff.out",
         "tags" : "12"
      },
      {
         "loc" : "fsme.auto",
         "tags" : "38"
      },
      {
         "loc" : "fsme.auto.in",
         "tags" : "56"
      },
      {
         "loc" : "fsme.auto.out",
         "tags" : "50"
      },
      {
         "loc" : "fsme.staff.in",
         "tags" : "291"
      },
      {
         "loc" : "fsme.staff.out",
         "tags" : "24"
      },
      {
         "loc" : "fsto.auto",
         "tags" : "254"
      },
      {
         "loc" : "fsto.auto.in",
         "tags" : "407"
      },
      {
         "loc" : "fsto.auto.out",
         "tags" : "643"
      },
      {
         "loc" : "fsto.staff",
         "tags" : "5"
      },
      {
         "loc" : "fsto.staff.in",
         "tags" : "465"
      },
      {
         "loc" : "fsto.staff.out",
         "tags" : "17"
      },
      {
         "loc" : "ftor.auto",
         "tags" : "209"
      },
      {
         "loc" : "ftor.auto.in",
         "tags" : "443"
      },
      {
         "loc" : "ftor.auto.out",
         "tags" : "868"
      },
      {
         "loc" : "ftor.staff",
         "tags" : "27"
      },
      {
         "loc" : "ftor.staff.in",
         "tags" : "713"
      },
      {
         "loc" : "ftor.staff.out",
         "tags" : "94"
      },
      {
         "loc" : "hsko.staff.in",
         "tags" : "66"
      },
      {
         "loc" : "hsko.staff.out",
         "tags" : "1"
      },
      {
         "loc" : "hutl.auto",
         "tags" : "782"
      },
      {
         "loc" : "hutl.auto.in",
         "tags" : "1504"
      },
      {
         "loc" : "hutl.auto.out",
         "tags" : "2182"
      },
      {
         "loc" : "hutl.inbox.1",
         "tags" : "0"
      },
      {
         "loc" : "hutl.staff",
         "tags" : "46"
      },
      {
         "loc" : "hutl.staff.in",
         "tags" : "2846"
      },
      {
         "loc" : "hutl.staff.out",
         "tags" : "88"
      },
      {
         "loc" : "hutl.yellow.21A",
         "tags" : "0"
      }
   ]
}

// GET /json/shelf/flam.pub
const mockShelfById = {
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
            }
         ]
      }
   ]
}

// GET /json/product/NO:02030000/34815
const mockProductById = {
   at: {
      epoch: 1517400746.49713,
      iso8601: "2018-01-31T12:12:26"
   },
   item_supplier: "NO:02030000",
   product_id: "34815",
   response: [
      {
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
            }
         ]
      }
   ]
}

const mockItemById = {
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
          location: "hutl.stack.foo",
          type: "CHECKOUT"
       },
       {
          action: null,
          at: {
             epoch: "1517321892.89213",
             iso8601: "2018-01-30T14:18:12"
          },
          dev: "5c:cf:7f:3a:37:45",
          location: "hutl.stack.bar",
          type: "CHECKIN"
       }
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
       }
    ]
  }
}

export { mockShelves, mockShelfById, mockProductById, mockItemById }
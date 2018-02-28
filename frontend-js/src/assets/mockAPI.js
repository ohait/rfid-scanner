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
         "loc" : "hutl.yellow.21A",
         "tags" : "0"
      }
   ]
}

// GET /json/shelf/flam.pub
const mockShelfById = {
  "at" : {
    "epoch" : 1519770317.82967,
    "in" : "now",
    "iso8601" : "2018-02-27T22:25:17"
  },
  "results" : [
    {
       "data" : {
          "author" : "Figueiredo, Ivo de",
          "biblionumber" : "9125571",
          "callnumber" : "CD BI,839.82,Fig,",
          "copynumber" : "2",
          "default_loc" : "hutl.pub.blue",
          "title" : "En fremmed ved mitt bord : familiefortelling"
       },
       "item_id" : "30119116607488",
       "item_supplier" : "NO:02030000",
       "last_at" : {
          "ago" : "4h",
          "epoch" : "1519755540.2478",
          "iso8601" : "2018-02-27T18:19:00"
       },
       "product_id" : "9125571",
       "tags" : [
          {
             "data" : {
                "base64" : ""
             },
             "permanent" : {
                "at" : null,
                "dev" : null,
                "loc" : null
             },
             "rfid" : "b:30119116607488",
             "temporary" : {
                "at" : {
                   "ago" : "4h",
                   "epoch" : "1519755540.2478",
                   "iso8601" : "2018-02-27T18:19:00"
                },
                "dev" : "fbje.bibliotheca_sip_proxy.10.172.17.31",
                "loc" : "fbje.auto"
             }
          }
       ]
    },
    {
       "data" : {
          "author" : "Nygaard, Olav",
          "biblionumber" : "571566",
          "callnumber" : ",510.2,Nyg,",
          "copynumber" : "10",
          "default_loc" : "fnor",
          "title" : "Fatte matte : for deg som vil tette huller i elementære matematikkunnskaper"
       },
       "item_id" : "03010571566010",
       "item_supplier" : "NO:02030000",
       "last_at" : {
          "ago" : "4h",
          "epoch" : "1519755539.30805",
          "iso8601" : "2018-02-27T18:18:59"
       },
       "product_id" : "571566",
       "tags" : [
          {
             "data" : {
                "base64" : ""
             },
             "permanent" : {
                "at" : null,
                "dev" : null,
                "loc" : null
             },
             "rfid" : "b:03010571566010",
             "temporary" : {
                "at" : {
                   "ago" : "4h",
                   "epoch" : "1519755539.30805",
                   "iso8601" : "2018-02-27T18:18:59"
                },
                "dev" : "fbje.bibliotheca_sip_proxy.10.172.17.31",
                "loc" : "fbje.auto"
             }
          }
       ]
    },
    {
       "data" : {
          "author" : "Thuve, Anne-Stine",
          "biblionumber" : "451826",
          "callnumber" : ",746.432,Thu,q",
          "copynumber" : "10",
          "default_loc" : "ftor",
          "title" : "Votteboken"
       },
       "item_id" : "03010451826010",
       "item_supplier" : "NO:02030000",
       "last_at" : {
          "ago" : "4h",
          "epoch" : "1519755538.55726",
          "iso8601" : "2018-02-27T18:18:58"
       },
       "product_id" : "451826",
       "tags" : [
          {
             "data" : {
                "base64" : ""
             },
             "permanent" : {
                "at" : null,
                "dev" : null,
                "loc" : null
             },
             "rfid" : "b:03010451826010",
             "temporary" : {
                "at" : {
                   "ago" : "4h",
                   "epoch" : "1519755538.55726",
                   "iso8601" : "2018-02-27T18:18:58"
                },
                "dev" : "fbje.bibliotheca_sip_proxy.10.172.17.31",
                "loc" : "fbje.auto"
             }
          }
       ]
    },
    {
       "data" : {
          "author" : "",
          "biblionumber" : "1126639",
          "callnumber" : "DVD,,Fis,",
          "copynumber" : "3",
          "default_loc" : "fbje",
          "title" : "En fisk ved navn Wanda"
       },
       "item_id" : "03011126639003",
       "item_supplier" : "NO:02030000",
       "last_at" : {
          "ago" : "2h",
          "epoch" : "1519762982.19344",
          "iso8601" : "2018-02-27T20:23:02"
       },
       "product_id" : "1126639",
       "tags" : [
          {
             "data" : {
                "base64" : ""
             },
             "permanent" : {
                "at" : null,
                "dev" : null,
                "loc" : null
             },
             "rfid" : "b:03011126639003",
             "temporary" : {
                "at" : {
                   "ago" : "2h",
                   "epoch" : "1519762982.19344",
                   "iso8601" : "2018-02-27T20:23:02"
                },
                "dev" : "fbje.bibliotheca_sip_proxy.10.172.17.32",
                "loc" : "fbje.auto"
             }
          }
       ]
    },
    {
       "data" : {
          "author" : "",
          "biblionumber" : "842533",
          "callnumber" : "DVD,,Usy,",
          "copynumber" : "2",
          "default_loc" : "fbje",
          "title" : "De Usynlige"
       },
       "item_id" : "03010842533002",
       "item_supplier" : "NO:02030000",
       "last_at" : {
          "ago" : "2h",
          "epoch" : "1519762980.75536",
          "iso8601" : "2018-02-27T20:23:00"
       },
       "product_id" : "842533",
       "tags" : [
          {
             "data" : {
                "base64" : ""
             },
             "permanent" : {
                "at" : null,
                "dev" : null,
                "loc" : null
             },
             "rfid" : "b:03010842533002",
             "temporary" : {
                "at" : {
                   "ago" : "2h",
                   "epoch" : "1519762980.75536",
                   "iso8601" : "2018-02-27T20:23:00"
                },
                "dev" : "fbje.bibliotheca_sip_proxy.10.172.17.32",
                "loc" : "fbje.auto"
             }
          }
       ]
    },
    {
       "data" : {
          "author" : "",
          "biblionumber" : "1412607",
          "callnumber" : "DVD,,Ann,",
          "copynumber" : "5",
          "default_loc" : "fbje",
          "title" : "Anna Karenina"
       },
       "item_id" : "03011412607005",
       "item_supplier" : "NO:02030000",
       "last_at" : {
          "ago" : "2h",
          "epoch" : "1519762979.4081",
          "iso8601" : "2018-02-27T20:22:59"
       },
       "product_id" : "1412607",
       "tags" : [
          {
             "data" : {
                "base64" : ""
             },
             "permanent" : {
                "at" : null,
                "dev" : null,
                "loc" : null
             },
             "rfid" : "b:03011412607005",
             "temporary" : {
                "at" : {
                   "ago" : "2h",
                   "epoch" : "1519762979.4081",
                   "iso8601" : "2018-02-27T20:22:59"
                },
                "dev" : "fbje.bibliotheca_sip_proxy.10.172.17.32",
                "loc" : "fbje.auto"
             }
          }
       ]
    },
    {
       "data" : {
          "author" : "",
          "biblionumber" : "1262082",
          "callnumber" : "DVD,,Nad,",
          "copynumber" : "17",
          "default_loc" : "fbje",
          "title" : "Nader og Simin, et brudd"
       },
       "item_id" : "30119116638408",
       "item_supplier" : "NO:02030000",
       "last_at" : {
          "ago" : "2h",
          "epoch" : "1519762978.06414",
          "iso8601" : "2018-02-27T20:22:58"
       },
       "product_id" : "1262082",
       "tags" : [
          {
             "data" : {
                "base64" : ""
             },
             "permanent" : {
                "at" : null,
                "dev" : null,
                "loc" : null
             },
             "rfid" : "b:30119116638408",
             "temporary" : {
                "at" : {
                   "ago" : "2h",
                   "epoch" : "1519762978.06414",
                   "iso8601" : "2018-02-27T20:22:58"
                },
                "dev" : "fbje.bibliotheca_sip_proxy.10.172.17.32",
                "loc" : "fbje.auto"
             }
          }
       ]
    },
    {
       "data" : {
          "author" : "",
          "biblionumber" : "1162069",
          "callnumber" : "DVD,,Nor,",
          "copynumber" : "5",
          "default_loc" : "fbje",
          "title" : "Norwegian wood"
       },
       "item_id" : "03011162069005",
       "item_supplier" : "NO:02030000",
       "last_at" : {
          "ago" : "2h",
          "epoch" : "1519762977.3348",
          "iso8601" : "2018-02-27T20:22:57"
       },
       "product_id" : "1162069",
       "tags" : [
          {
             "data" : {
                "base64" : ""
             },
             "permanent" : {
                "at" : null,
                "dev" : null,
                "loc" : null
             },
             "rfid" : "b:03011162069005",
             "temporary" : {
                "at" : {
                   "ago" : "2h",
                   "epoch" : "1519762977.3348",
                   "iso8601" : "2018-02-27T20:22:57"
                },
                "dev" : "fbje.bibliotheca_sip_proxy.10.172.17.32",
                "loc" : "fbje.auto"
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
   "at" : {
      "epoch" : 1519772054.88027,
      "in" : "now",
      "iso8601" : "2018-02-27T22:54:14"
   },
   "item_id" : "30119116663833",
   "item_supplier" : "NO:02030000",
   "response" : {
      "history" : [
         {
            "actions" : [
               "info"
            ],
            "at" : {
               "ago" : "16h",
               "epoch" : "1519715873.5278",
               "iso8601" : "2018-02-27T07:17:53"
            },
            "dev" : "fbje.bibliotheca_sip_proxy.10.172.17.31",
            "rfid" : "b:30119116663833"
         },
         {
            "actions" : [
               "check out"
            ],
            "at" : {
               "ago" : "3d",
               "epoch" : "1519487975.86776",
               "iso8601" : "2018-02-24T15:59:35"
            },
            "dev" : "fbje.bibliotheca_sip_proxy.10.172.17.31",
            "rfid" : "b:30119116663833"
         }
      ],
      "item_id" : "30119116663833",
      "item_supplier" : "NO:02030000",
      "last_seen" : {
         "ago" : "16h",
         "epoch" : "1519715873.10282",
         "iso8601" : "2018-02-27T07:17:53"
      },
      "meta" : {
         "author" : "Gaiman, Neil",
         "biblionumber" : "9134541",
         "callnumber" : "u,293.1,Gai,",
         "copynumber" : "17",
         "default_loc" : "fbje",
         "title" : "Norrøne guder : fra Yggdrasil til ragnarok"
      },
      "product_id" : "9134541",
      "tags" : [
         {
            "data" : {
               "base64" : ""
            },
            "permanent" : {
               "at" : null,
               "dev" : null,
               "loc" : null
            },
            "rfid" : "b:30119116663833",
            "temporary" : {
               "at" : {
                  "ago" : "16h",
                  "epoch" : "1519715873.05947",
                  "iso8601" : "2018-02-27T07:17:53"
               },
               "dev" : "fbje.bibliotheca_sip_proxy.10.172.17.31",
               "loc" : "fbje.auto"
            }
         }
      ]
   }
}

export { mockShelves, mockShelfById, mockProductById, mockItemById }
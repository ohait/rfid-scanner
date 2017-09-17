#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Fonts/Picopixel.h>
#include <Fonts/FreeSans9pt7b.h>

//#include <Wire.h>
#include "secrets.h"
#include <ESP8266WiFi.h>
WiFiClient client;
#include <Adafruit_SSD1305.h>
Adafruit_SSD1305 display(2);

#include "jmy6xx.h";

#include <SoftwareSerial.h>
SoftwareSerial SSerial(12, 14); // RX, TX to the RFID module


JMY6xx rfid(&SSerial);
//JMY6xx rfid(0xa0);

byte mac[8];
void setup() {
  pinMode(2, OUTPUT);
  Serial.begin(9600);
  SSerial.begin(19200);

  display.clearDisplay();
  display.display();

  delay(50);


  rfid.debug = 1;
  delay(200);
  Serial.println(String("INIT... build ")+__DATE__+" "+__TIME__);
  rfid.info();

  display.begin();
  display.setRotation(2);
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.macAddress(mac);

  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Hello World!");
  
  display.setFont(&Picopixel);
  display.println("Hello World!");
  

  Serial.println("INIT DONE");
}

byte data[1024];
void loop() {
  digitalWrite(0, LOW);
  byte* b1 = data;
  byte* b2 = data;
  for(b1=data;b1<data+1024;b1+=8) {
    const byte* uid = rfid.scan();
    if (!uid) break;
    Serial.print("found TAG UID: ");
    hexprint(uid, 8);
    Serial.println();
    memcpy(b1, uid, 8);    
    rfid.quiet();
  }

  for(b2=data;b2<b1;b2+=8) {
    rfid.ready(b2);
  }
  
  display.clearDisplay();
  display.setFont();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("millis: ");
  display.println(millis());
  display.println("test");
  display.println("last");
  display.display();

  digitalWrite(0, HIGH);
  delay(50);
}

void hexprint(const byte* data, int length) {
  for (int i=0; i<length; i++) {
    if (i==0) Serial.print(data[i] < 0x10 ? "0" : "");
    else Serial.print(data[i] < 0x10 ? ":0" : ":");
    Serial.print(data[i], HEX);
  }
}

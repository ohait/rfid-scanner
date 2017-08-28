#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Fonts/Picopixel.h>
#include <Adafruit_SSD1305.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
WiFiClient client;

#include <jmy6xx.h>;
//JMY6xx jmy622(&SSerial);
JMY6xx jmy622(0x50); // 0xA0/0xA1 => 0x50 in arduino world

Adafruit_SSD1305 display(2);

#define BUZZER_PIN 15
#define CHECKIN_PIN 13

#include "sounds.h"
#include "secrets.h"

// IDLE
long idle_expire = 1000*60;

// SHELF
char shelf[32] = "fake.shelf\0";
long shelf_expire = 0;

// DISPLAY MESSAGES
int display_prio = 0;
long display_expire = 0;
int scan_count = 0;

// CURRENT STATE
int state = 1; // 0 idle, 1 Inventory, 2 Checkin

void checkin() {
  if (digitalRead(CHECKIN_PIN)==LOW) {
    state = 2;
  } else {
    state = 1;
  }
  idle_expire = millis()+1000*60;
  expire = 0;
//  Serial.print("checking() => ");
//  Serial.println(state);
}


////////////////////////////////////////////
// UPDATE DISPLAY
////////////////////////////////////////////

void update_display() {
  if (millis() < display_expire) return;
  display_prio = 0;
//  expire = millis()+1000;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont();
  display.setCursor(0,0);
  if (state==1) {
    display.println("Inventory");
  } else if (state==2) {
    display.println("Check IN");
  } else {
    display.println(String("State: ")+state);
  }
  
  display.setFont(&Picopixel);
  display.setCursor(0,30);
  int wifi = wifi_connect();
  display.println(String("")+" WiFi: "+wifi+"%"+" shelf: "+shelf);
  display.display();
}

void error(String msg) {
  if (display_prio >= 9) return;
  display_prio = 9;
  display_expire = millis()+1000*20;
  toneKO();
  Serial.println(msg);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont();
  display.setCursor(0,0);
  display.println(msg);
  display.setFont(&Picopixel);
  display.display();
}

void pick(String cn, String barcode, String author, String title) {
  if (display_prio >= 5) return;
  display_prio = 5;
  display_expire = millis()+1000*10;
  tonePICK();
  Serial.println(String("PICK ")+barcode);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont();
  display.setCursor(0,0);
  display.println("PICK "+barcode);
  display.println(author);
  display.println(title);
  
  display.display();
}

void info(String s) {
  if (display_prio >= 1) return;
  display_prio = 1;
  display_expire = millis()+1000*20;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont();
  display.setCursor(0,0);
  display.println(s);
  display.display();
}

int read_blocks = 7;
byte* queue;
int q_size = 1024;
int q_pos = 0;
int record_size = 8+2+read_blocks*4+2; // align 

byte* queue_find(const byte* uid) {
  Serial.print("queue search ");
  jmy622.hexprint(uid, 8);
  Serial.println();
  
  for(int p=0; p<q_pos;) {
    byte* record = queue+p;

    Serial.print("pos: 0x");
    if (p<16) Serial.print("000");
    else if (p<256) Serial.print("00");
    else if (p<16*256) Serial.print("0");
    Serial.print(p, HEX);
    Serial.print(": ");
    jmy622.hexprint(record, 8);
    
    if (memcmp(uid, record, 8)) {
      Serial.println(" NO");
      p += record_size;
    } else {
      Serial.println(" FOUND");
      return record;
    }
  }
  Serial.println("Not in queue");
  return NULL;
}

int scan() {
  if (q_pos >= q_size - record_size) {
    error("Queue full");
    return 0;
  }
  int ct = 0;
  for(; ct<100; ct++) {
    const byte* uid = jmy622.iso15693_scan();
    
    if (!uid) break;

    byte* record = queue_find(uid);
    if (record) {
    } else {
      record = queue + q_pos;
      memcpy(record, uid, 8);
      q_pos += record_size;
      Serial.print("New tag: ");
      jmy622.hexprint(uid, 8);
      Serial.print(", queue is ");
      Serial.print(q_pos);
      Serial.print("/");
      Serial.println(q_size);
      scan_count++;
    }

    const byte* data = jmy622.iso15693_read(0,read_blocks); // read blocks 0..7

    jmy622.hexprint(uid, 8); 
    Serial.print(" => ");
    Serial.println((long)data, HEX);
    
    if (!data) break;
    
    
    record[8] = 0; // flag TODO checkin button
    record[9] = read_blocks*4+2; // align
    
    memcpy(record+10, data, record_size-10);
      
    jmy622.iso15693_quiet();
    
    Serial.print("found tag: ");
    jmy622.hexprint(record, 8);
    Serial.println();
    jmy622.hexdump(record+10, 7*4); // 4 bytes per block
    if (state==2) {
       toneTock();
    } else {
       toneTick();
    }

    String s = String("");
    for(int i=0; i<7*4; i++) {
      char c = (char)record[i];
      if (c>=32 and c<127) {
        s = s+c;
      } else {
        s = s+'.';
      }
    }
    info(s);
  }
  return ct;
}

void setup() {
  Serial.begin(9600);
  //Serial.begin(38400);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(CHECKIN_PIN, INPUT_PULLUP);
  //jmy622.debug = 3;

  toneOK();
  Serial.println("scanning I2C");
  for(int addr=0; addr<127; addr++) {
    delay(10);
    Wire.beginTransmission(addr);
    int error = Wire.endTransmission();
    if (error==0) {
      Serial.print("device found at 0x");
      toneTick();
      Serial.println(addr, HEX);
    }
  }
  Serial.println("scan done");
  toneOK();


  delay(200);

  attachInterrupt(digitalPinToInterrupt(CHECKIN_PIN), checkin, CHANGE);
  display.begin();

  WiFi.mode(WIFI_STA);
//  WiFi.macAddress(mac);

  delay(200);


  queue = (byte*)malloc(q_size);
  if (!queue) {
    error("Can't allocate minumum queue memory");
    while(1) delay(1000);
  }
  for(;;) {
    int size = q_size*2;
    byte* q = (byte*)realloc(queue, size);
    if (!q) break;
    queue = q; q_size = size;
  }
  
  Serial.println();
  Serial.println(String("INIT... build ")+__DATE__+" "+__TIME__);
  Serial.print(String("queue ")+(q_size/1024)+"Kb (0x"); Serial.print((long)queue, HEX); Serial.println();

  display.clearDisplay();
  display.setRotation(2);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont(&Picopixel);
  display.setCursor(0,4);
  display.println(String("INIT... built ")+__DATE__+" - "+__TIME__);
  display.display();

  jmy622.info();
  toneOK();
}


void loop() {
    
  if (millis() > idle_expire) {
    state = 0;
    idle_expire = millis() + 1000*60; // reset the timer

    display.clearDisplay();
    display.display();

    jmy622.idle();
  }

  delay(50);

  if (state==0) {
    WiFi.mode(WIFI_OFF);
    delay(500);
    return;
  }
  WiFi.mode(WIFI_STA);

  if (scan()) {
    Serial.println("-------------");
    idle_expire = millis()+1000*60;
    //pick("123.45 Dew", "03010034815003", "U. N. Owen", "Misterious Title, with subtitle");
  } else {
    update_display();
  }
}

long wifi_timeout = 0;
int wifi_connect() {
  if (WiFi.status() == WL_CONNECTED) {
      int rssi = WiFi.RSSI();
      rssi = rssi < -90 ? 0 : rssi > -50 ? 100 : (rssi+90)*100/(90-50);
      return rssi;
  }

  if (millis()<wifi_timeout) {
    Serial.println("still connecting...");
    return 0;
  }

  display.clearDisplay();
  display.setFont();  
  display.setTextSize(1);
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.println("Seaching for WiFi...");
  Serial.println("Seaching for WiFi...");
  display.display();

  int ct = WiFi.scanNetworks();
  for (int i=0; i<ct; i++) {
    String ssid = WiFi.SSID(i);
    const char* pwd = wifi_pass(ssid.c_str());
    if (!pwd) continue;
    int rssi = WiFi.RSSI(i);
    rssi = rssi < -90 ? 0 : rssi > -50 ? 100 : (rssi+90)*100/(90-50);
    if (rssi<10) continue;

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.setTextColor(WHITE);
    display.println(String("Found: ")+ssid+" "+rssi+"%");
    Serial.println(String("Found: ")+ssid+" "+rssi+"%");
    display.display();

    WiFi.begin(ssid.c_str(), pwd);
    wifi_timeout = millis() + 1000*20; // wait 20 seconds before trying another network
    return 0;
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.println("No valid wifi networks");
  Serial.println("No valid wifi networks");
  display.display();
  wifi_timeout = millis() + 1000*20; // try again in 20 seconds
  return 0;
}


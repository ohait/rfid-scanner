#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Fonts/Picopixel.h>
#include <Adafruit_SSD1305.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
WiFiClient client;

#define IDLE_TIME 30

#include <SoftwareSerial.h>
#include <jmy6xx.h>;
SoftwareSerial SSerial(12,14);
JMY6xx jmy622(&SSerial);
//JMY6xx jmy622(0x50); // 0xA0/0xA1 => 0x50 in arduino world

Adafruit_SSD1305 display(2);

#define BUZZER_PIN 15
#define CHECKIN_PIN 13

#include "sounds.h"
#include "secrets.h"

// IDLE
long idle_expire = 1000*IDLE_TIME;

// SHELF
char shelf[32] = "fake.shelf\0";
long shelf_expire = 0;

// DISPLAY MESSAGES
int display_prio = 0;
long display_expire = 0;
int scan_count = 0;

// CURRENT STATE
int state = 1; // 0 idle, 1 Inventory, 2 Checkin

int wake = 0;
void checkin() {
  if (state==0) wake=1;
  if (digitalRead(CHECKIN_PIN)==LOW) {
    state = 2;
  } else {
    state = 1;
  }
  idle_expire = millis()+1000*IDLE_TIME;
//  Serial.print("checking() => ");
//  Serial.println(state);
}


////////////////////////////////////////////
// UPDATE DISPLAY
////////////////////////////////////////////

int wifi_gfx[] = {
  0,0,
  3,0,  3,1,  2,2,
  6,0,  6,1,  6,2,  5,3,  5,4,
  9,0,  9,1,  9,2,  8,3,  8,4,  7,5,
};

int display_cycle = 0;
void update_display() {
  if (millis() < display_expire) return;
  display_prio = 0;
//  expire = millis()+1000;

  yield();
  display.clearDisplay();

  display_cycle++;
  int wx = 127;
  int wy = 0;

  if (client.connected()) {
    display_cycle %=4;
    if (display_cycle<2) {
      display.drawLine(wx-4,wy, wx-4,wy+4, WHITE);
      display.drawLine(wx-5,wy+1,wx-3, wy+1, WHITE);
    } 
    if (display_cycle==1 or display_cycle==2) {
      display.drawLine(wx-1,wy, wx-1,wy+4, WHITE);
      display.drawLine(wx-2,wy+3,wx-0, wy+3, WHITE);
    }
  } else {
    int wifi = wifi_connect()*14/100;
    if (wifi>13) wifi=13;
    if (wifi<0) wifi=0;  
    display_cycle %= wifi+2;
    for (int i=0; i<display_cycle; i++) {
      display.drawPixel(wx-wifi_gfx[i*2], wy+wifi_gfx[i*2+1], WHITE);
    }
  }
  yield();
  
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
  

  if (shelf[0]) {
    display.setFont(&Picopixel);
    display.setCursor(0,30);
    display.println(String("")+"shelf: "+shelf);
  }

/*
  if (wifi>99) wifi=99;
  display.setCursor(100,30);
  display.println(String("")+"WiFi:"+wifi+"%");
  */
  yield();
  display.display();
}

void error(String msg) {
  if (display_prio >= 9) return;
  display_prio = 9;
  display_expire = millis()+1000*20;
  toneKO();
  Serial.println(msg);

  yield();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont();
  display.setCursor(0,0);
  display.println(msg);
  display.setFont(&Picopixel);
  yield();
  display.display();
}

void pick(String cn, String barcode, String author, String title) {
  if (display_prio >= 5) return;
  display_prio = 5;
  display_expire = millis()+1000*10;
  tonePICK();
  Serial.println(String("PICK ")+barcode);

  yield();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont();
  display.setCursor(0,0);
  display.println("PICK "+barcode);
  display.println(author);
  display.println(title);
  yield();
  display.display();
}

void info(String s) {
  if (display_prio >= 1) return;
  display_prio = 1;
  display_expire = millis()+1000*20;

  yield();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont();
  display.setCursor(0,0);
  display.println(s);
  yield();
  display.display();
}

int read_blocks = 7;
byte* send_buffer;
byte* queue;
int q_size = 1024;
int q_pos = 0;
int record_size = 8+2+read_blocks*4+2; // align 

byte* queue_find(const byte* uid) {
  Serial.print("queue search ");
  jmy622.hexprint(uid, 8);
  Serial.println();
  
  for(int p=0; p<q_pos;) {
    yield();
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
    yield();
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
  SSerial.begin(19200);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(CHECKIN_PIN, INPUT_PULLUP);
//  jmy622.debug = 3;

  toneOK();
  delay(100);
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


  yield();
  delay(100);

  attachInterrupt(digitalPinToInterrupt(CHECKIN_PIN), checkin, CHANGE);
  display.begin();

  WiFi.mode(WIFI_STA);
//  WiFi.macAddress(mac);

  yield();
  delay(100);

  send_buffer = (byte*)malloc(q_size);
  if (!send_buffer) {
    error("Can't allocate minumum queue memory");
    while(1) delay(1000);
  }
  for(;;) {
    int size = q_size*2;
    byte* q = (byte*)realloc(send_buffer, size);
    if (!q) break;
    send_buffer = q; q_size = size;
  }
  queue = send_buffer+10;
  q_size -= 10;

  yield();
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

void sleep() {
  state = 0;
  yield();
  WiFi.mode(WIFI_OFF);
//  yield();
//  jmy622.idle();
  yield();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont();
  display.setCursor(0,0);
  display.println("Zzzz....");
  display.display();
  yield();
  delay(1000);
  yield();
  display.command(SSD1305_DISPLAYOFF);
  yield();
}
void wake_up() {
  toneOK();
  yield();
  display.command(SSD1305_DISPLAYON);
  display.clearDisplay();
  display.display();
  
  yield();
  WiFi.mode(WIFI_STA);
  yield();
  idle_expire = millis()+1000*IDLE_TIME;
}

void loop() {
  // IDLE LOOP?
  if (state==0) {
    delay(100);
    return;
  }

  if (wake) {
    wake_up(); 
    wake=0;
  }
  if (millis() > idle_expire) {
    sleep();
    return;
  }

  if (scan()) {
    Serial.println("-------------");
    idle_expire = millis()+1000*IDLE_TIME;
    //pick("123.45 Dew", "03010034815003", "U. N. Owen", "Misterious Title, with subtitle");
  } else {
    update_display();
    wifi_recv();
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
    return 0;
  }


  yield();
  int ct = WiFi.scanNetworks();
  for (int i=0; i<ct; i++) {
    yield();
    String ssid = WiFi.SSID(i);
    const char* pwd = wifi_pass(ssid.c_str());
    if (!pwd) continue;
    int rssi = WiFi.RSSI(i);
    rssi = rssi < -90 ? 0 : rssi > -50 ? 100 : (rssi+90)*100/(90-50);
    if (rssi<10) continue;

    WiFi.begin(ssid.c_str(), pwd);
    wifi_timeout = millis() + 1000*20; // wait 20 seconds before trying another network
    return 0;
  }
  wifi_timeout = millis() + 1000*20; // try again in 20 seconds
  return 0;
}

long wifi_wait = 0;
int wifi_send() {
  if (!q_pos) return 0; // nothing to do
  if (client.connected()) { // already waiting for an answer
    return 0;
  }
  if (wifi_connect()<30) { // poor wifi
    return 0;
  }
  if (millis()<wifi_wait) { // I should wait
    return 0;
  }
  wifi_wait = millis()+1000*1; // antiflood

  int len = q_pos+8;
  if (!client.connect(host(), port())) {
    Serial.println(String("client.connect(")+host()+":"+port()+") failed");
    error("server connection failed");
    return 0;
  }
  //Serial.println(String("Sending data ")+millis());
  client.print(String("POST ") + url() + " HTTP/1.0\r\n" +
               "Host: " + host() + "\r\n" + 
               "Content-Length: "+ (len) +"\r\n" +
               "Connection: close\r\n\r\n");

  int sent = 0;
  while(sent < len) {
    int w = client.write((const byte*)(send_buffer+sent), len-sent);
    if (w<1) {
      error("server send failed");
      wifi_wait = millis()+1000*15; // 15 seconds wait after errors
      return 0;
    }
    sent += w;
  }
  Serial.print("sent "); Serial.print(sent); Serial.println(" bytes");
  wifi_timeout = millis()+1000*15; 
}

int wifi_recv() {
  if (!client.connected()) return 0;
  if (millis() > wifi_timeout) {
    error("timeout waiting from server");
    client.stop();
    return 0;
  }
  if (!client.available()) return 0;

  Serial.println("RECV");

  String cmd = client.readStringUntil('\n');
  Serial.println(cmd);
  while(client.available()) {
    yield();
    String line = client.readStringUntil('\n');
    if (line.length()<2) break;
    Serial.println(line);
  }
  Serial.println("..");

  String msg = client.readStringUntil('\n');
  Serial.println(msg);

  if (msg.equals("WRT")) {
    toneWAIT();
    int len = client.readStringUntil('\n').toInt();
//    memset(wid, 0, 256);
//    client.read(wid, len);
//    SERIALHEXDUMP(wid, len);
                           // 0x00    0C    00    5F  E3 DB CF 19 00 00 07 E0  chk: 5A
  }
  else if (msg.equals("NOOP")) {
    String barcode = client.readStringUntil('\n');
    String author = client.readStringUntil('\n');
    String title = client.readStringUntil('\n');
    String cn = client.readStringUntil('\n');
    info(barcode+"\n"+author+"\n"+title);
  }
  else if (msg.equals("PICK")) {
    String barcode = client.readStringUntil('\n');
    String author = client.readStringUntil('\n');
    String title = client.readStringUntil('\n');
    String cn = client.readStringUntil('\n');
    pick(cn, barcode, author, title);
  } else {
    Serial.println(String("UNKNOWN CMD: '")+msg+"'");
  }

  return 1;
}


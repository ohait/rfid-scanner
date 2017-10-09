#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Fonts/Picopixel.h>
#include <Adafruit_SSD1305.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
WiFiClient client;

#define IDLE_TIME 300

#include <SoftwareSerial.h>
#include <jmy6xx.h>;
#define RFID_RX 12
#define RFID_TX 14
SoftwareSerial SSerial(RFID_RX,RFID_TX);
JMY6xx jmy622(&SSerial);
//JMY6xx jmy622(0x50); // 0xA0/0xA1 => 0x50 in arduino world

Adafruit_SSD1305 display(2);

#define BUZZER_PIN 15
#define CHECKIN_PIN 13

#include "sounds.h"
#include "secrets.h"

// IDLE
long idle_after = 1000*IDLE_TIME;

// GLOBAL VARS
int start_block = 0;
int read_blocks = 9;
byte* send_buffer;
byte* queue;
int q_size = 24*1024;
int q_pos = 0;
int record_size = 8+2+read_blocks*4; // align
char shelf[32];
long shelf_expire = 0;

// DISPLAY MESSAGES
int display_prio = 0;
long display_expire = 0;

int count_total = 0;
int count_from_idle = 0;
int count_shelf = 0;

// CURRENT STATE
int state = 1; // 0 idle, 1 Inventory, 2 Checkin
long epoch = 0;
int wake = 0;
long prev_checkin = 0;
void checkin() {
  if (state==0) wake=1;
  if (digitalRead(CHECKIN_PIN)==LOW) { // PRESS
    prev_checkin = millis();
    state = 2;
  } else {
    long ago = millis() - prev_checkin;
    prev_checkin = 0;
    state = 1;
    if (ago<500 && ago>10) { // sometimes buttons are bouncing, so only at least 50 ms long
      shelf[0] = '\0'; // a quick press will reset the shelf
    }
  }
  idle_after = millis()+1000*IDLE_TIME; // extends the idle time
  display_expire = 0; // clear the display
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
  if (millis() > display_expire) full_update_display();
  display_cycle++;

  yield();
  int wx = 127;
  int wy = 0;
  display.fillRect(wx-10, wy, 11, 7, BLACK);
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
  display.display();
}

void full_update_display() {
  display_prio = 0;
  yield();
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

  yield();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont(&Picopixel);
  display.setCursor(0,14);
  display.print(count_total);
  display.print(" tags scanned");
  if (shelf[0]) {
    display.print(", ");
    display.print(count_shelf);
    display.print(" tags on shelf");
  }
  else if (count_from_idle != count_total) {
    display.print(", ");
    display.print(count_from_idle);
    display.print(" since idle");
  }

  yield();

  if (shelf[0]) { // first byte is not \0
    display.setFont(&Picopixel);
    display.setCursor(0,22);
    display.println(String("")+"shelf: "+shelf);
  }

  yield();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont(&Picopixel);
  display.setCursor(0,30);
  int queue_full = q_pos *100 /q_size;
  display.print("buffer: ");
  display.print(q_pos/record_size);
  display.print(" (");
  display.print(queue_full);
  display.print("%)");

  if (epoch) {
    display.setFont(&Picopixel);
    display.setCursor(110,30);
    long e = millis()/1000+epoch;
    struct tm* now = localtime(&e);
    display.print(now->tm_hour);
    display.print(now->tm_sec%2 ? " ":":");
    display.print(now->tm_min<10?"0":"");
    display.print(now->tm_min);
  }

/*
  if (wifi>99) wifi=99;
  display.setCursor(100,30);
  display.println(String("")+"WiFi:"+wifi+"%");
  */
  yield();
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


byte* queue_find(const byte* uid) {
  //Serial.print("queue search "); jmy622.hexprint(uid, 8); Serial.println();
  
  for(int p=0; p<q_pos;) {
    yield();
    byte* record = queue+p;

//    Serial.print("pos: 0x");
//    if (p<16) Serial.print("000");
//    else if (p<256) Serial.print("00");
//    else if (p<16*256) Serial.print("0");
//    Serial.print(p, HEX);
//    Serial.print(": ");
//    jmy622.hexprint(record, 8);
    
    if (memcmp(uid, record, 8)) {
      //Serial.println(" NO");
      p += record_size;
    } else {
      //Serial.println(" FOUND");
      return record;
    }
  }
//  Serial.println("Not in queue");
  return NULL;
}

int scan() {
  byte buf[read_blocks*4];
  if (q_pos >= q_size - record_size) {
    error("Queue full");
    return 0;
  }
  int ct = 0;
  for(; ct<100; ct++) {
    yield();
    const byte* uid = jmy622.iso15693_scan();
    
    if (!uid) break;

    yield();
    const byte* data = jmy622.iso15693_read(start_block,read_blocks);
    if (!data) break;
    yield();
    memcpy(buf, data, read_blocks*4);
    // iso_quiet() will overwrite the data, so let's have a safe copy

    jmy622.hexprint(uid, 8); Serial.print(" => "); Serial.println((long)data, HEX);
    
    if (!jmy622.iso15693_quiet()) break;

    byte* record = queue_find(uid);
    if (record) {
    } else {
      record = queue + q_pos;
      memcpy(record, uid, 8);
      q_pos += record_size;
//      Serial.print("New tag: ");
//      jmy622.hexprint(uid, 8);
//      Serial.print(", queue is ");
//      Serial.print(q_pos);
//      Serial.print("/");
//      Serial.println(q_size);
      count_total++;
      count_from_idle++;
      count_shelf++;
    }

    record[8] = 0; // flag TODO checkin button
    record[9] = read_blocks*4; // align

    memcpy(record+10, buf, read_blocks*4);

    if (state==2) {
       toneTock();
    } else {
       toneTick();
    }
    
//    Serial.println("record: ");
//    jmy622.hexdump(record, record_size);

    for (int i=10; i<record_size-10; i++) {
//      hexdump(record+i, strlen("SHELF#"));
      if (memcmp((char*)(record+i), "SHELF#", strlen("SHELF#"))) continue;
      i+= strlen("SHELF#");
      strncpy(shelf, (char*)(record+i), 32);
      Serial.println(shelf);
      count_shelf = 0;
    }
  }
  return ct;
}

void setup() {
  //Serial.begin(9600);
  Serial.begin(115200);
  SSerial.begin(19200);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(CHECKIN_PIN, INPUT_PULLUP);
  pinMode(A0, INPUT);
//  jmy622.debug = 3;

  toneOK();
  delay(200);

  attachInterrupt(digitalPinToInterrupt(CHECKIN_PIN), checkin, CHANGE);
  display.begin();

  WiFi.mode(WIFI_STA);
//  WiFi.macAddress(mac);

  yield();
  delay(100);

  yield();
  Serial.println();
  Serial.println(String("INIT... build ")+__DATE__+" "+__TIME__);

  send_buffer = (byte*)malloc(q_size);
  if (!send_buffer) {
    error("Can't allocate minumum queue memory");
    while(1) delay(1000);
  }

  send_buffer[0] = 0x42;
  send_buffer[1] = 0x42;
  WiFi.macAddress(send_buffer+2);
  send_buffer[8] = '\0'; // shelf
  shelf[0] = '\0';
  queue = send_buffer+8+32;
  q_size -= 8+32;
  Serial.println(String("queue ")+(q_size/1024)+"Kb");

  display.clearDisplay();
  display.setRotation(2);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont(&Picopixel);
  display.setCursor(0,4);
  display.print("INIT... built ");
  display.print(__DATE__);
  display.print(" - ");
  display.println(__TIME__);
  display.display();

  jmy622.info();
  toneOK();

}

void sleep() {
  state = 0;
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
  shelf[0] = '\0';
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
  idle_after = millis()+1000*IDLE_TIME;
  count_from_idle = 0;
}

void loop() {
  // IDLE LOOP?
  if (state==0) {
    delay(1000);
    return;
  }

  if (wake) {
    wake_up(); 
    wake=0;
  }
  if (millis() > idle_after) {
    sleep();
    return;
  }

  if (scan()) {
    //Serial.println("-------------");
    idle_after = millis()+1000*IDLE_TIME;
  } else {
    update_display();
    wifi_recv();
    wifi_send();
    wifi_init();
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

#ifdef WIFI_SSID
#ifdef WIWI_PASS
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  wifi_timeout = millis() + 1000*20; // wait 20 seconds before trying another network
  return 0;
#endif
#endif

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
long next_ping = 0;
int wifi_send() {
  if (!q_pos && millis() < next_ping) return 0; // nothing to do
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

  int len = q_pos+8+32;
  if (!client.connect(host(), port())) {
    Serial.println(String("client.connect(")+host()+":"+port()+") failed");
    error("server connection failed");
    return 0;
  }
  next_ping = millis()+1000*60;
  //Serial.println(String("Sending data ")+millis());
  client.print(String("POST ") + url() + " HTTP/1.0\r\n" +
               "Host: " + host() + "\r\n" + 
               "Content-Length: "+ (len) +"\r\n" +
               "Connection: close\r\n\r\n");

//  Serial.println("SENDING:");
//  jmy622.hexdump(send_buffer, len);
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
  wifi_timeout = millis()+1000*15; // read timeout
  q_pos = 0;
  memcpy(send_buffer+8, shelf, 32); // mark for the next data
}

long wifi_init_wait = 0;
int wifi_init() {
  if (client.connected()) { // already waiting for an answer
    return 0;
  }
  if (wifi_connect()<30) { // poor wifi
    return 0;
  }
  return 1;
  if (millis()<wifi_init_wait) { // I should wait
    return 0;
  }
  wifi_init_wait = millis()+1000*300; // antiflood
  if (!client.connect(host(), port())) {
    Serial.println(String("client.connect(")+host()+":"+port()+") failed");
    error("server connection failed");
    return 0;
  }
  client.print(String("GET ") + url() + " HTTP/1.0\r\n" +
               "Host: " + host() + "\r\n" + 
               "Connection: close\r\n\r\n");
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

  for(;;) {
    String msg = client.readStringUntil('\n');
    if (!msg) break;
    Serial.println(msg);

    if (msg.equals("WRT")) {
      toneWAIT();
      int len = client.readStringUntil('\n').toInt();
//    memset(wid, 0, 256);
//    client.read(wid, len);
//    SERIALHEXDUMP(wid, len);
    }
    else if (msg.equals("LOCAL_EPOCH")) {
      String e = client.readStringUntil('\n');
      epoch = e.toInt()-millis()/1000;
    }
    else if (msg.equals("IMG") or msg.equals("PIMG")) {
      if (msg.equals("PIMG")) tonePICK();
      display.clearDisplay();
      for (int i=0; i<6; i++) {
        delay(1);
        String line = client.readStringUntil('\n');
        for (int x=0; x<128; x++) {
          char c = line[x];
          byte b = 0;
          if (c>='A' && c<='Z') b = c-'A';
          else if (c>='a' && c<='z') b = c-'a'+26;
          else if (c>='0' && c<='9') b = c-'0'+52;
          else if (c=='+') b = 62;
          else b = 63;
          delay(0);
          //Serial.print(b<16 ? ":0":":");
          //Serial.print(b, HEX);
          for (int y=0; y<6; y++) {
            if (b&(1<<y)) display.drawPixel(x, y+i*6, WHITE);
            else  display.drawPixel(x, y+i*6, BLACK);
          }
        }
        //Serial.println();
      }
      display.display();
      display_prio = 5;
      display_expire = millis()+1000*20;
    }
    else if (msg.equals("END")) {
      break;
    }
    else {
      Serial.println(String("UNKNOWN CMD: '")+msg+"'");
      wifi_wait = millis()+1000*30;
      break;
    }
  }
  client.stop();
  
  return 1;
}


/*
 * Feather HUZZAH Wifi
 * 
 *           |USB|
 *     RST*  \   / +  
 *     3v3*        /--
 *       -*         BATTERY
 *     GND*        \--
 *      17*         *VBAT
 *       -*         *En
 *       -*         *VBUS
 *       -*         *14
 *       -*         *12
 *       -*         *13
 * TX===14*         *15
 * buz==13*         *0
 * RX===12*         *16
 *       3*         *2
 *       1*         *5
 *     CHP*         *4
 *  
 * SoftSerial speak with the RFID chip using 12,14
 * Buzzer use pin 13 to give user feedback
 * 
 * TODO: use OLED FeatherWing, will be I2C on 4/5 (SDA/SCL)
 */

//#define DEBUG
// fancy debug on serial port
#ifdef DEBUG
#define SERIALDEBUG(line) Serial.println(String("")+millis()+" "+line); //*/
#define SERIALHEXDUMP(out, len) serialhexdump(out, len); //*/
#else
#define SERIALDEBUG(line) //*/
#define SERIALHEXDUMP(out, len) //*/
#endif

#include <ESP8266WiFi.h>
WiFiClient client;
int client_work = 0;
long wifi_timeout = 0;

#include <SoftwareSerial.h>
SoftwareSerial RFID(12, 14); // RX, TX to the RFID module
byte inventory[] = { 0x00, 0x04, 0x00, 0x5C };
byte read_block[] = { 0x00, 0x06, 0x00, 0x54, 0, 9 };
byte stay_quiet[] = { 0x00, 0x04, 0x00, 0x5d };

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 3
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#include "secrets.h"
const char* ssid_pwd(const char* ssid) {
  if (strcmp(ssid, "oha.it")==0) return OHAIT_P;
  if (strcmp(ssid, "hovedbib")==0) return HOVEDBIB_P;
  return 0;
}

char* _host     = "beta.oha.it";
int   _port     = 80;
char* _url      = "/arduino/rfid/";

const char* host() {
  if (WiFi.SSID().equals("hovedbib")) return HOVEDBIB_HOST;
  return _host;
}

const char* hhost() { // host name sent in headers, overriding hostname used to connect
  //if (WiFi.SSID().equals("hovedbib")) return "foo";
  return host();
}

int port() {
  //if (WiFi.SSID().equals("hovedbib")) return 80;
  return _port;
}

const char* url() {
  return _url;
}


byte mac[6];

#define QUEUE_SIZE (40*500)
// QUEUE of tags found, ready to send
byte* _queue = (byte*)malloc(QUEUE_SIZE+40);
byte* queue = _queue+40;
byte* pos = queue;

// buffer for IO
byte* out = (byte*)malloc(1024);

// to write
byte* wid = (byte*)malloc(256);
#define TAG_READ_SIZE (8)
byte* rid = (byte*)malloc(8*TAG_READ_SIZE);

// shelf
char* shelf = (char*)malloc(32);
long shelf_expire = 0;

volatile int scan_mode = 1;
void btnA() {
  scan_mode = !scan_mode;
  shelf_expire = 1;
}

void setup() {
  Serial.begin(9600);
  RFID.begin(19200);
  memset(wid, 0, 256);
  memset(rid, 0, 8*TAG_READ_SIZE);
  strcpy(shelf, "");
  delay(200);
  pinMode(0, INPUT); digitalWrite(0, HIGH);
  attachInterrupt(digitalPinToInterrupt(0), btnA, RISING);
//  pinMode(2, INPUT); digitalWrite(2, HIGH);
//  attachInterrupt(digitalPinToInterrupt(16), btnB, RISING);
//  pinMode(16, INPUT); digitalWrite(16, HIGH);
//  attachInterrupt(digitalPinToInterrupt(2), btnC, RISING);
  
  Serial.println(String("INIT... build ")+__DATE__+" "+__TIME__);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Deichman");
  display.setTextSize(1);
  display.println("Shelf Cleaner");
  display.println(String("build ")+__DATE__);
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.macAddress(mac);

  byte info[] = { 0x00, 0x04, 0x00, 0x10};
  if(rfid_req(info, out)>10) {
    //toneACK();
  } else {
    Serial.println("Can't find the RFID reader");
    toneKO();
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("NO RFID MODULE");
    display.display();
    while(1) delay(60000);
  }
  delay(1000);
  wifi_connect();
  //toneOK();
}

long display_expire = 0;
int display_level = 0;
void loop() {
  delay(5);
  if (shelf_expire==0) {
    delay(200);
  }
  long t0 = millis();
  if (shelf_expire && t0>shelf_expire) {
    memset(shelf, 0, 32);
    shelf_expire = 0;
    toneNO();
  }
  
  if (t0 > display_expire) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.setTextColor(WHITE);
    display.println(shelf);
    display.println();
    if (WiFi.status() == WL_CONNECTED) {
      display.print(String("SSID ")+WiFi.SSID());
      int rssi = WiFi.RSSI();
      rssi = rssi < -90 ? 0 : rssi > -50 ? 100 : (rssi+90)*100/(90-50);
      display.println(String(" ")+rssi+"%");
    } else if (wifi_timeout) {
      display.println(String("Connecting"));
    } else {
      display.println(String("OFFLINE "));
    }
    display.println(String("queue: ")+((pos-queue)/40)+"/"+(QUEUE_SIZE/40)+" "+(scan_mode?"NORM":"FAST"));
    display.display();
    display_level = 0;
  }

  int i;
  for (i=0; i<100; i++) {
    int r = rfid_req(inventory, out);
    if (r<8) break;

    // current tag needs to be written
    if (memcmp(out+5, wid, 8)==0) {
      write_tag();
      return;
    }

    Serial.print("RFID: "); hex2str(out+5, 8); Serial.println();

    // should I read the tag blocks?
    int must_read = scan_mode;
    if (WiFi.status() != WL_CONNECTED) must_read++;
    if (!must_read) {
      for (int i=0; i<TAG_READ_SIZE; i++) {
        if (memcmp(out+5, rid+8*i, 8)==0) {
          must_read++;
          memset(rid+8*i, 0, 8); // remove
          break;
        }
      }
    }
    if (is_queued(out+5)) {
      must_read = 0;
    }
    if (must_read) {
      memcpy(pos, out+5, 8); // rfid tag id
      pos[8] = 0; // flags TODO if button, send check-in
      pos += 9;
      int len = rfid_req(read_block, out);
      //SERIALHEXDUMP(out, len);

      if (len>128) { // IGNORE
      } else if (len<4) { // IGNORE
      } else if (memcmp(out+4, "SHELF#", 6)==0) {
        pos[-1] = 128; // raise shelf flag
        pos[0] = len-4; pos++;
        memcpy(pos, out+4, len-4); // skip first 4 bytes (len+frame+cmd)
        pos += len-4;

        memcpy(shelf, out+4+6, 32-6);
        shelf_expire = millis() + 1000*60;
        // TODO fetch more data, and search for SSID/PWD
        toneOK();
      }
      else {
        pos[0] = len-4; pos++;
        memcpy(pos, out+4, len-4); // skip first 4 bytes (len+frame+cmd)
        pos += len-4;
        if (shelf_expire) {
          shelf_expire = millis() + 1000*60;
          toneTick();
        } else {
          toneTock();
        }
      }
    } 
    else { // only the tag id will be sent
      memcpy(pos, out+5, 8); // rfid tag id
      pos[8] = 0; // flags
      pos[9] = 0; // length
      pos += 10;
      toneTick();
    }
    // tell the CURRENT_TAG to stay quiet (until it exits the field)
    // so we can scan other tags
    rfid_req(stay_quiet, out);
  }

  if (i>0) { // if 1 or more tags were scanned, just go back scanning for more!
    //Serial.println(String("SCANNED ")+i+" ITEM. "+(millis()-t0));
    return;
  }

  // no queue and client_work
//  if (pos==queue && !client_work) {
//    //Serial.println(String("NOTHING MORE ")+(millis()-t0));
//    wifi_connect();
//    delay(50);
//    return; // nothing to send
//  }

  send_queue(); // deque
}

int is_queued(const byte* rfid) {
  SERIALDEBUG("is_queued");
  SERIALHEXDUMP(rfid, 8);

  for (byte* p=queue; p<pos; ) {
    SERIALDEBUG(String("check pos: ")+(p-queue));
    SERIALHEXDUMP(p,8);
    if (p[8]&128) { // SHELF IGNORE
    } else if (memcmp(p, rfid, 8)==0) {
      return 1;
    }
    int len = p[9];
    SERIALDEBUG(String("skip: 10+")+(len));
    p+= 10+len;
  }
  return 0;
}

void write_tag() {
  Serial.print("WRITING "); hex2str(wid, 8); Serial.println();
  int len = 4*12;
  out[0] = 0; out[1] = 6+len; // len
  out[2] = 0; // frame
  out[3] = 0x55; // cmd
  out[4] = 0; // start block
  out[5] = len/4; // size
  memcpy(out+6, wid+8, len); // write data to wid
  int l = rfid_req(out, out);
  memset(wid, 0, 256);
  delay(5);
}

int wifi_connect() {
  if (wifi_timeout > 0) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected");
      wifi_timeout = 0;
      return 1;
    }
    else if (millis() > wifi_timeout) {
      Serial.println("WiFi timeout");
      wifi_timeout = 0;
    } else {
      //Serial.println(".");
      return 0;
    }
  }
  if (WiFi.status() == WL_CONNECTED) return 1;

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.print("Seaching for WiFi...");
  display.display();

  int ct = WiFi.scanNetworks();
  for (int i=0; i<ct; i++) {
    String ssid = WiFi.SSID(i);
    Serial.println(String("available ")+ssid);
    const char* pwd = ssid_pwd(ssid.c_str());
    if (pwd) {
      Serial.println(String("Connecting to: ")+ssid);
      WiFi.begin(ssid.c_str(), pwd);
      wifi_timeout = millis() + 1000*20;
      return 0;
    }
  }
  Serial.println("no valid networks");  
}

void send_queue() {
  long t0 = millis();
  if (client.connected()) {
    // don't block unless there is the first byte
    if (!client.available()) {
      SERIALDEBUG("nothing to read");
      return;
    }
    SERIALDEBUG("READ");
    // blocking now
    String cmd = client.readStringUntil('\n');
    SERIALDEBUG(cmd);
  
    while(client.available()) {
      String line = client.readStringUntil('\n');
      SERIALDEBUG(line);
      if (line.length()<2) break;
    }

    SERIALDEBUG("MSG: ");
    String msg = client.readStringUntil('\n');
    SERIALDEBUG(msg);

    if (msg.equals("WRT")) {
      toneWAIT();
      int len = client.readStringUntil('\n').toInt();
      SERIALDEBUG("RFID WRITE len: "+len);
      memset(wid, 0, 256);
      client.read(wid, len);
      SERIALHEXDUMP(wid, len);
                             // 0x00    0C    00    5F  E3 DB CF 19 00 00 07 E0  chk: 5A
      byte reset_to_ready[] = { 0x00, 0x0C, 0x00, 0x5F, 1, 2, 3, 4, 5, 6, 7, 8 };
      memcpy(reset_to_ready+4, wid, 8);
      SERIALHEXDUMP(reset_to_ready, 12);
      int l = rfid_req(reset_to_ready, out);
      SERIALHEXDUMP(out, l);
    }
    else if (msg.equals("READ")) {
      toneTock();
      SERIALDEBUG("RFID READ");
      // shift the list, and prepend the new id
      memcpy(rid+8, rid, 8*(1-TAG_READ_SIZE));
      client.read(rid, 8);

      byte reset_to_ready[] = { 0x00, 0x0C, 0x00, 0x5F, 1,2,3,4,5,6,7,8 };
      memcpy(reset_to_ready+4, rid, 8);
      SERIALHEXDUMP(reset_to_ready, 12);
      int l = rfid_req(reset_to_ready, out);
      SERIALHEXDUMP(out, l);
    }
    else if (msg.equals("NOOP")) {
      String barcode = client.readStringUntil('\n');
      String author = client.readStringUntil('\n');
      String title = client.readStringUntil('\n');
      String cc = client.readStringUntil('\n');
      String loc = client.readStringUntil('\n');

      if (display_level<=0 && !barcode.equals("")) { // don't overwrite PICK or similar
        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.println(loc);
        display.print(barcode);
        display.print(" ");
        display.println(cc);
        display.println(author);
        display.println(title);
        display.display();
        display_expire = millis()+5000;
        display_level = 0;
      }
    }
    else if (msg.equals("PICK")) {
      String barcode = client.readStringUntil('\n');
      String author = client.readStringUntil('\n');
      String title = client.readStringUntil('\n');
      String cc = client.readStringUntil('\n');
      String loc = client.readStringUntil('\n');

      Serial.println("PICK "+barcode);
  
      display.clearDisplay();
      display.setCursor(0,0);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      //display.println(cc);
      display.println(loc);
      display.print(barcode);
      display.print(" ");
      display.println(cc);
      display.println(author);
      display.println(title);
      display.display();
      display_expire = millis()+5000;
      display_level = 2;
      
      tonePICK();
    } else {
      Serial.println(String("UNKNOWN CMD: '")+msg+"'");
    }
    SERIALDEBUG("READ DONE");
    client_work = 0;
  } else if (pos > queue) {
    if (wifi_connect()==0) {
      SERIALDEBUG("Waiting for connection");
      return;
    }
    int rssi = WiFi.RSSI();
    rssi = rssi < -90 ? 0 : rssi > -50 ? 100 : (rssi+90)*100/(90-50);
    if (rssi < 10) {
        SERIALDEBUG("Connection is poor");
        return;
    }
    int len = pos-queue;
    display.drawLine(127,31,127,31,WHITE);
    display.display();
    SERIALDEBUG("Sending "+len+" bytes");

    //if (!client.connect(host, port)) {
    if (!client.connect(host(), port())) {
      Serial.println(String("client.connect(")+host()+":"+port()+") failed");
      toneKO();
      display.clearDisplay();
      display.setCursor(0,0);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.println("server conn fail");
      display.display();
      delay(5*1000);
      return;
    }
    //Serial.println(String("Sending data ")+millis());
    client.print(String("POST ") + url() + " HTTP/1.0\r\n" +
                 "Host: " + hhost() + "\r\n" + 
                 "Content-Length: "+ (40+len) +"\r\n" +
                 "Connection: close\r\n\r\n");

    // HEAD
    _queue[0] = 0x42;
    _queue[1] = 0x42;
    memcpy(_queue+2, mac, 6); // device mac address
    memcpy(_queue+8, shelf, 32); // rfid tag id
    //Serial.println("send"); SERIALHEXDUMP(_queue, 40+len);
    client.write((const byte*)_queue, 40+len);
    SERIALDEBUG("request sent");
    pos = queue;
    client_work = 1;
  }
  else {
    //SERIALDEBUG("NOOP");
  }
}

int rfid_timeout = 0;
byte rfid_read() {
  if(rfid_timeout) return 0;
  for (int i=0; i<500/5; i++) {
    if (RFID.available()) {
      return RFID.read();
    }
    delay(5);
  }
  rfid_timeout=1;
  Serial.println("Timeout");
  return 0;
}

int rfid_req(const byte* req, byte* out) {
  rfid_timeout = 0;
  byte rs = 0;
  int rlen = req[0]*256+ req[1];

  for (int i=0; i<rlen; i++) {
    RFID.write(req[i]);
    rs = rs ^ req[i];
  }
  RFID.write(rs);
  
  out[0] = rfid_read();
  out[1] = rfid_read();
  byte sum = out[0] ^ out[1];
  int len = out[0]*256 + out[1];
  for (int i=2; i<len; i++) {
    byte b = rfid_read();
    out[i] = b;
    sum = sum ^ b;
  }
  byte chk = out[len] = rfid_read();
  if (sum != chk) {
    Serial.print("got "); Serial.print(len); Serial.print(" bytes, checksum: "); Serial.print(sum, HEX); Serial.print(" expected: "); Serial.print(chk, HEX); Serial.println();
    return 0;
  }
  return len;
}


void toneOK() {
  tone(13, 6000); delay(40);
  tone(13, 12000); delay(30);
  noTone(13);
}

void toneNO() {
  tone(13, 30); delay(150);
  noTone(13);
}

void toneTick() {
  tone(13, 12000); delay(30);
  noTone(13);
}

void toneTock() {
  tone(13, 8000); delay(20);
  noTone(13);
}

void tonePICK() {
  tone(13, 500);  delay(50);
  noTone(13);     delay(20);
  tone(13, 500);  delay(50);
  noTone(13);     delay(20);
  tone(13, 500);  delay(50);
  noTone(13);
}

void toneWAIT() {
  tone(13, 3000);  delay(100);
  tone(13, 500);  delay(200);
  noTone(13);
}

void toneKO() {
  tone(13, 2000);  delay(30);
  tone(13, 250);  delay(50);
  tone(13, 1500);  delay(30);
  tone(13, 250);  delay(50);
  tone(13, 1000);  delay(30);
  tone(13, 250);  delay(50);
  tone(13, 2000);  delay(30);
  noTone(13);
}


void hex2str(const byte* data, int length) {
  for (int i=0; i<length; i++) {
    if (i==0) Serial.print(data[i] < 0x10 ? "0" : "");
    else Serial.print(data[i] < 0x10 ? " 0" : " ");
    Serial.print(data[i], HEX);
  }
}

void serialhexdump(const byte* data, int length) {
  for (int i=0; i<length; i+=8) {
    if (i<10) Serial.print("  ");
    else if (i<100) Serial.print(" ");
    Serial.print(i);
    Serial.print(":");
    for (int j=i; j<i+8; j++) {
      if (j>=length) {
        Serial.print("   ");
      } else {
        Serial.print(data[j] < 0x10 ? " 0" : " ");
        Serial.print(data[j], HEX);
      }
    }
    Serial.print("  \"");
    for (int j=i; j<length && j<i+8; j++) {
      if (data[j]>=0x20 && data[j]<=0x7E) Serial.write(data[j]);
      else Serial.print(".");
    }
    Serial.println("\"");
  }
}

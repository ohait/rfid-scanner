#include <jmy6xx.h>;
JMY6xx jmy622(0x50); // 0xA0/0xA1 => 0x50 in arduino world

#define BUZZER_PIN 15
#define CHECKIN_PIN 13
#include "sounds.h"

char cbuf[256];
const char* hex2str(const byte* data, int len) {
  if (len>256/3) len = 256/3;
  char *s = cbuf;
  for (int i=0; i<len; i++) {
    if (i>0) { *s = ':'; s++; }
    int h = data[i]/16;
    int l = data[i]%16;
    if (h<10) *s = '0'+h;
    else *s = 'a'+h-10;
    s++;
    if (l<10) *s = '0'+l;
    else *s = 'a'+l-10;
    s++;
  }
  *s='\0';
  return cbuf;
}

// IDLE
long idle_expire = 1000*60;

// SHELF
char shelf[32] = "fake.shelf\0";
long shelf_expire = 0;

// DISPLAY MESSAGES
int prio = 0;
long expire = 0;
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

void update_display() {
  if (millis() < expire) return;
//  expire = millis()+1000;
}

void error(String msg) {
  toneKO();
  Serial.println(msg);
  prio = 9;
  expire = millis()+1000*20;
}

void pick(String cn, String barcode, String author, String title) {
  tonePICK();
  Serial.println(String("PICK ")+barcode);
  prio = 9;
  expire = millis()+1000*10;
}

int read_blocks = 7;
byte* queue;
int q_size = 1024;
int q_pos = 0;
int record_size = 8+2+read_blocks*4;

byte* queue_find(const byte* uid) {
  for(int p=0; p<q_pos;) {
    byte* record = queue+p;
    if (memcmp(uid, record, 8)) {
      p += record[9]||1; // length
    } else {
      return record;
    }
  }
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
    if (!record) {
      record = queue + q_pos;
      memcpy(record, uid, 8);
      q_pos += record_size;
      Serial.println("New tag, queue is ");
      Serial.print(q_pos);
      Serial.print("/");
      Serial.println(q_size);
      scan_count++;
    }
    
    record[8] = 0; // flag TODO checkin button
    record[9] = record_size;
    
    const byte* data = jmy622.iso15693_read(0,read_blocks); // read blocks 0..7
    if (data) {
      memcpy(record+10, data, read_blocks*4);
      
      if (jmy622.iso15693_quiet()) { // set the current tag to quiet, so we can scan others in range
        Serial.print("found tag: ");
        jmy622.hexprint(record, 8);
        Serial.println();
        jmy622.hexdump(record+10, 7*4); // 4 bytes per block
        if (state==2) {
          toneTock();
        } else {
          toneTick();
        }
      } else {
        Serial.println("can't stay quiet tag");
      }
    }
    else { // no data read
      memset(record+10, 0xff, read_blocks*4);
      Serial.print("found tag, but no data: ");
      jmy622.hexprint(record, 8);
      Serial.println();
    }
    
  }
  return ct;
}


void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(CHECKIN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CHECKIN_PIN), checkin, CHANGE);
  //jmy622.debug = 3;
  
  delay(100);

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

  toneOK();
  jmy622.info();
}


void loop() {
    
  if (state && millis() > idle_expire) {
    state = 0;
    idle_expire = millis() + 1000*60; // reset the timer
    jmy622.idle();
    toneOUT();
  }

  if (state==0) return;

  if (scan()) {
    Serial.println("-------------");
    idle_expire = millis()+1000*60;
    //pick("123.45 Dew", "03010034815003", "U. N. Owen", "Misterious Title, with subtitle");
  } else {
    delay(50);
    update_display();
  }
}


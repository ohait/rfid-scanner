#ifndef SOUNDS_H
#define SOUNDS_H

#ifndef BUZZER_PIN
#error #define BUZZER_PIN _before_ #include "sounds.h"
#endif

inline void toneOK() {
  tone(BUZZER_PIN, 6000); delay(30);
  tone(BUZZER_PIN, 12000); delay(40);
  noTone(BUZZER_PIN);
}

inline void toneNO() {
  tone(BUZZER_PIN, 30); delay(150);
  noTone(BUZZER_PIN);
}

inline void toneTick() {
  tone(BUZZER_PIN, 8000); delay(30);
  noTone(BUZZER_PIN);
}

inline void toneTock() {
  tone(BUZZER_PIN, 200); delay(30);
  noTone(BUZZER_PIN);
}

inline void tonePICK() {
  tone(BUZZER_PIN, 500);  delay(50);
  noTone(BUZZER_PIN);     delay(20);
  tone(BUZZER_PIN, 500);  delay(50);
  noTone(BUZZER_PIN);     delay(20);
  tone(BUZZER_PIN, 500);  delay(50);
  noTone(BUZZER_PIN);
}

inline void toneWAIT() {
  tone(BUZZER_PIN, 3000);  delay(100);
  tone(BUZZER_PIN, 500);  delay(200);
  noTone(BUZZER_PIN);
}

inline void toneKO() {
  tone(BUZZER_PIN, 2000);  delay(30);
  tone(BUZZER_PIN, 250);  delay(50);
  tone(BUZZER_PIN, 1500);  delay(30);
  tone(BUZZER_PIN, 250);  delay(50);
  tone(BUZZER_PIN, 1000);  delay(30);
  tone(BUZZER_PIN, 250);  delay(50);
  tone(BUZZER_PIN, 2000);  delay(30);
  noTone(BUZZER_PIN);
}

inline void toneOUT() {
  tone(BUZZER_PIN, 800);  delay(50);
  tone(BUZZER_PIN, 400);  delay(50);
  tone(BUZZER_PIN, 200);  delay(50);
  noTone(BUZZER_PIN);
}

#endif

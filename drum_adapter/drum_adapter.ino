#include "Keyboard.h"

#define CENTER_PIN        A0
#define RIM_PIN           A1
#define CENTER_THRESHOLD  70
#define RIM_THRESHOLD     7
#define RIM_CENTER_CUTOFF 40
#define CENTER_KEY_1      106 // j
#define CENTER_KEY_2      102 // f
#define RIM_KEY_1         100 // d
#define RIM_KEY_2         107 // k
#define HIT_LENGTH_MS     15
#define SAMPLE_DELAY_US   50
#define NUM_SAMPLES       40
//#define DOUBLE_HIT
#define DEBUG_MESSAGES

int rim, center, temp, i;

void setup() {
  #ifdef DEBUG_MESSAGES
  Serial.begin(9600);
  #endif
  Keyboard.begin();
}

void loop() {
  rim = center = temp = 0;
  for (i = 0; i < NUM_SAMPLES; i++) {
    temp = analogRead(RIM_PIN); 
    rim += abs(512 - temp);
    temp = analogRead(CENTER_PIN);
    center += abs(512 - temp);
    delayMicroseconds(SAMPLE_DELAY_US);
  }
  
  rim /= NUM_SAMPLES;
  center /= NUM_SAMPLES;

  if (rim > RIM_THRESHOLD && center < RIM_CENTER_CUTOFF) {
    #ifdef DEBUG_MESSAGES
    Serial.print("Rim Hit! Rim raw = ");
    Serial.print(rim);
    Serial.print(", Center raw = ");
    Serial.println(center);
    #endif
    
    Keyboard.press(RIM_KEY_1);
    #ifdef DOUBLE_HIT
    Keyboard.press(RIM_KEY_2);
    #endif
    
    delay(HIT_LENGTH_MS);
    
    Keyboard.release(RIM_KEY_1);
    #ifdef DOUBLE_HIT
    Keyboard.release(RIM_KEY_2);
    #endif
  } else if (center > CENTER_THRESHOLD) {
    #ifdef DEBUG_MESSAGES
    Serial.print("Center Hit! Rim raw = ");
    Serial.print(rim);
    Serial.print(", Center raw = ");
    Serial.println(center);
    #endif
    
    Keyboard.press(CENTER_KEY_1);
    #ifdef DOUBLE_HIT
    Keyboard.press(CENTER_KEY_2);
    #endif
    
    delay(HIT_LENGTH_MS);
    
    Keyboard.release(CENTER_KEY_1);
    #ifdef DOUBLE_HIT
    Keyboard.release(CENTER_KEY_2);
    #endif
  } 
}

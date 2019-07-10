#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, 8, NEO_GRB + NEO_KHZ800);

int seq[6][15] = {  
   {0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0} , 
   {0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1} , 
   {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1} , 
   {1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0} , 
   {0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0} , 
   {1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1}
};

int pins[6] = {2, 3, 6, 9, 10, 12};

int vibeDelay = 0;
int currStep = 0;
int currTime = 0;
int state = 0;

void setup() {
  for (int i = 0; i < 6; i++) {
    pinMode(pins[i], OUTPUT);
  }

  strip.begin();
  strip.setBrightness(50);
  strip.show(); // Initialize all pixels to 'off'

  vibeDelay = random(500);
}

void loop() {
  colorWipe(strip.Color(255, 0, 0), 500); // Red
  colorWipe(strip.Color(0, 255, 0), 500); // Green
  colorWipe(strip.Color(0, 0, 255), 500); // Blue
  rainbowCycle(20);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}
 
// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;
 
  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
    currTime += wait;

    if (currTime >= vibeDelay && state == 0) {
      for (int i = 0; i < 6; i++) {
        digitalWrite(pins[i], seq[i][currStep % 15]);
      }
      currStep++;
      state = 1;
    }
    else if (currTime >= 300 && state == 1) {
       for (int i = 0; i < 6; i++) {
        digitalWrite(pins[i], 0);
      }
      state = 2;
    }
    else if (currTime >= 1000 && state == 2) {
      currTime = 0;
      state = 0;
      vibeDelay = random(500);
    }
  }
}
 
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

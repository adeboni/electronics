#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_PM25AQI.h"
#include <Bounce2.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Bounce button = Bounce();

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

int mode = 4;

void setup() {
  button.attach(2, INPUT_PULLUP);
  button.interval(5);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;);
  }

  display.clearDisplay();

  Serial.begin(9600);
  if (!aqi.begin_UART(&Serial)) {
    showError();
    while (1) delay(10);
  }
}

void showError() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(F("Sensor error"));
  display.display();
}

void loop() {
  button.update();
  if (button.fell()) {
    mode = (mode + 1) % 5;
  }

  PM25_AQI_Data data;
  if (!aqi.read(&data))
    return;

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 8);
  
  if (mode == 0) {
    display.print(F("PM1.0: "));
    display.print(data.pm10_standard);
  } else if (mode == 1) {
    display.print(F("PM2.5: "));
    display.print(data.pm25_standard);
  } else if (mode == 2) {
    display.print(F("PM4.0: "));
    display.print(data.pm40_standard);
  } else if (mode == 3) {
    display.print(F("PM10: "));
    display.print(data.pm100_standard);
  } else if (mode == 4) {
    display.print(F("AQI: "));
    display.print(getAQI(data.pm25_standard));
  }
  display.display();

  delay(100);
}

uint16_t getAQI(uint16_t pm) {
  if (pm > 350.5)
    return calcAQI(pm, 500, 401, 500, 350.5);
  else if (pm > 250.5)
    return calcAQI(pm, 400, 301, 350.4, 250.5);
  else if (pm > 150.5)
    return calcAQI(pm, 300, 201, 250.4, 150.5);
  else if (pm > 55.5)
    return calcAQI(pm, 200, 151, 150.4, 55.5);
  else if (pm > 35.5)
    return calcAQI(pm, 150, 101, 55.4, 35.5);
  else if (pm > 12.1)
    return calcAQI(pm, 100, 51, 35.4, 12.1);
  else if (pm > 0)
    return calcAQI(pm, 50, 0, 12, 0);
  else return 9999;
}

uint16_t calcAQI(uint16_t cp, float ih, float i1, float bph, float bp1) {
  float a = ih - i1;
  float b = bph - bp1;
  float c = cp - bp1;
  return (uint16_t)((a/b)*c+i1);
}

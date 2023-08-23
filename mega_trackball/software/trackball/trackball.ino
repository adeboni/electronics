#include <Encoder.h>
#include "Mouse.h"

#define LEFT_BTN   4
#define MIDDLE_BTN 5
#define RIGHT_BTN  6

#define RANGE 5
#define DELAY 10

Encoder encX(2, 3);
Encoder encY(0, 1);


long xPos = -999;
long yPos = -999;

void setup() {
  pinMode(LEFT_BTN, INPUT_PULLUP);
  pinMode(MIDDLE_BTN, INPUT_PULLUP);
  pinMode(RIGHT_BTN, INPUT_PULLUP);

  xPos = encX.read();
  yPos = encY.read();
}

void loop() {
  if (digitalRead(LEFT_BTN) == LOW && !Mouse.isPressed(MOUSE_LEFT))
     Mouse.press(MOUSE_LEFT);
  else if (digitalRead(LEFT_BTN) == HIGH && Mouse.isPressed(MOUSE_LEFT))
    Mouse.release(MOUSE_LEFT);

  if (digitalRead(RIGHT_BTN) == LOW && !Mouse.isPressed(MOUSE_RIGHT))
     Mouse.press(MOUSE_RIGHT);
  else if (digitalRead(RIGHT_BTN) == HIGH && Mouse.isPressed(MOUSE_RIGHT))
    Mouse.release(MOUSE_RIGHT);

  if (digitalRead(MIDDLE_BTN) == LOW && !Mouse.isPressed(MOUSE_MIDDLE))
     Mouse.press(MOUSE_MIDDLE);
  else if (digitalRead(MIDDLE_BTN) == HIGH && Mouse.isPressed(MOUSE_MIDDLE))
    Mouse.release(MOUSE_MIDDLE);

  long newX = encX.read();
  long newY = encY.read();

  if (newX != xPos || newY != yPos)
    Mouse.move(newX - xPos, newY - yPos, 0);
  xPos = newX;
  yPos = newY;
  
  delay(DELAY);
}

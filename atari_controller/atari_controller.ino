#define X_PIN 3
#define Y_PIN 2

#define DOWN_PIN 7
#define RIGHT_PIN 5
#define UP_PIN 8
#define LEFT_PIN 6

int threshold = 200;
int mode = 0;

void setup() {
  pinMode(UP_PIN, OUTPUT);
  pinMode(DOWN_PIN, OUTPUT);
  pinMode(LEFT_PIN, OUTPUT);
  pinMode(RIGHT_PIN, OUTPUT);

  int x = analogRead(X_PIN);
  int y = analogRead(Y_PIN);

  if (x < 100)
    mode = 1;
  else if (x > 900)
    mode = 2;
  else 
    threshold = y < 512 ? y : 1023 - y;
}

void loop() {
  int x = analogRead(X_PIN);
  int y = analogRead(Y_PIN);

  switch (mode) {
    case 0:
      mode0(x, y);
      break;
    case 1:
      mode1(x, y);
      break;
    case 2:
      //mode2(x, y);
      break;
  }
}

void mode0(int x, int y) {
  digitalWrite(LEFT_PIN, x < threshold ? HIGH : LOW);
  digitalWrite(RIGHT_PIN, x > (1023 - threshold) ? HIGH : LOW);
  digitalWrite(DOWN_PIN, y < threshold ? HIGH : LOW);
  digitalWrite(UP_PIN, y > (1023 - threshold) ? HIGH : LOW);
  delay(50);
}

void mode1(int x, int y) {
  int x2, y2;
  
  if (x < 512) {
    x2 = map(x, 512, 0, 0, 50);
    digitalWrite(LEFT_PIN, HIGH);
  }
  else {
    x2 = map(x, 512, 1023, 0, 50);
    digitalWrite(RIGHT_PIN, HIGH);
  }
  
  if (y < 512) {
    y2 = map(y, 512, 0, 0, 50);
    digitalWrite(DOWN_PIN, HIGH);
  }
  else {
    y2 = map(y, 512, 1023, 0, 50);
    digitalWrite(UP_PIN, HIGH);
  }

  for (int i = 0; i <= 50; i++) {
    if (i == x2) {
      digitalWrite(LEFT_PIN, LOW);
      digitalWrite(RIGHT_PIN, LOW);
    }

    if (i == y2) {
      digitalWrite(DOWN_PIN, LOW);
      digitalWrite(UP_PIN, LOW);
    }

    delay(1);
  }
}

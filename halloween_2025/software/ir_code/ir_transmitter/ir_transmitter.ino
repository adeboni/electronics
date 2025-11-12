#define MOTOR_1_PIN    5
#define MOTOR_2_PIN    6
#define NEOPIXEL_PIN   7
#define IR_RECEIVE_PIN 4
#define IR_SEND_PIN    8
#define TRIGGER_PIN    17
#define SD_CLK_PIN     10
#define SD_CMD_PIN     11
#define SD_DAT3_PIN    13
#define SD_DAT2_PIN    14
#define SD_DAT1_PIN    15
#define SD_DAT0_PIN    12
#define PCM_DAT_PIN    26
#define PCM_BCK_PIN    27
#define PCM_LRCLK_PIN  28
#define INPUT_VOLT_PIN 29
#define MOTOR_ON_TIME_MS 100
#include <IRremote.hpp>
#include <SPI.h>
#include <SD.h>
#include <AudioFileSourceSD.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2S.h>
#include <Bounce2.h>

Bounce trigger = Bounce();

uint8_t idPins[3] = {2, 1, 0};
uint8_t id = 0;

unsigned long motorStartTime = 0;
bool motorOn = false;

File file;
AudioFileSourceSD *sdSource;
AudioGeneratorWAV *wav;
AudioOutputI2S *out;

void setup() {
  pinMode(INPUT_VOLT_PIN, INPUT);
  for (int i = 0; i < 3; i++)
    pinMode(idPins[i], INPUT_PULLUP);
  pinMode(MOTOR_1_PIN, OUTPUT);
  pinMode(MOTOR_2_PIN, OUTPUT);
  analogWrite(MOTOR_1_PIN, 0);
  analogWrite(MOTOR_2_PIN, 0);

  trigger.attach(TRIGGER_PIN, INPUT_PULLUP);
  trigger.interval(10);

  Serial.begin(115200);

  sdSource = new AudioFileSourceSD();
  wav = new AudioGeneratorWAV();
  wav->SetBufferSize(1024);
  out = new AudioOutputI2S();
  out->SetPinout(PCM_BCK_PIN, PCM_LRCLK_PIN, PCM_DAT_PIN);
  out->SetGain(1.0);

  SPI1.setRX(SD_DAT0_PIN);
  SPI1.setTX(SD_CMD_PIN);
  SPI1.setSCK(SD_CLK_PIN);
  SPI1.setCS(SD_DAT3_PIN);
  SD.begin(SD_DAT3_PIN, SPI_FULL_SPEED, SPI1);

  getID();
}

void loop() {
  checkButton();

  if (motorOn && millis() - motorStartTime > MOTOR_ON_TIME_MS) {
    motorOn = false;
    analogWrite(MOTOR_1_PIN, 0);
  }

  if (wav->isRunning())
    if (!wav->loop())
      wav->stop();
}

void checkButton() {
  trigger.update();
  if (trigger.changed() && trigger.read() == LOW) {
    Serial.println("Trigger pressed!");
    sdSource->close();
    IrSender.sendNEC(0x00, 0x34 + id, 0);
    if (sdSource->open("laser.wav")) 
      wav->begin(sdSource, out);
    motorOn = true;
    motorStartTime = millis();
    analogWrite(MOTOR_1_PIN, 255);
  }
}

uint8_t getID() {
  uint8_t result = 0;
  for (int i = 0; i < 3; i++)
    if (digitalRead(idPins[i]) == LOW)
      result |= 1 << i;
  id = result;
  return result;
}

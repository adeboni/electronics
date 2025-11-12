#define MOTOR_1_PIN    5
#define MOTOR_2_PIN    6
#define NEOPIXEL_PIN   7
#define IR_RECEIVE_PIN 4
#define IR_SEND_PIN    8
#define TRIGGER_PIN    17
#define ULTRA_TX       18
#define ULTRA_RX       20
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
#define MOTOR_ON_TIME_MS 50
#include <IRremote.hpp>
#include <SPI.h>
#include <SD.h>
#include <AudioFileSourceSD.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2S.h>
#include <NewPing.h>
#include <Adafruit_NeoPixel.h>

File file;
AudioFileSourceSD *sdSource;
AudioGeneratorWAV *wav;
AudioOutputI2S *out;
bool playClick = false;

NewPing distanceSensor(ULTRA_TX, ULTRA_RX, 200);
Adafruit_NeoPixel pixel(1, NEOPIXEL_PIN, NEO_RGB + NEO_KHZ800);

float frequency = 1.0;
float cycleTime = 1.0;
float stepDelay = 10.0;

// Color to pulse
uint8_t red = 52;
uint8_t green = 235;
uint8_t blue = 88;

unsigned long motorStartTime = 0;
bool motorOn = false;

void setup() {
  pinMode(INPUT_VOLT_PIN, INPUT);
  pinMode(MOTOR_1_PIN, OUTPUT);
  pinMode(MOTOR_2_PIN, OUTPUT);
  analogWrite(MOTOR_1_PIN, 0);
  analogWrite(MOTOR_2_PIN, 0);

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
}

void loop() {
  unsigned int distance = distanceSensor.ping_cm();
  frequency = (200 - distance) / 10.0;
  cycleTime = 1.0 / frequency;
  stepDelay = (cycleTime / 512) * 1000.0;
  
  if (motorOn && millis() - motorStartTime > MOTOR_ON_TIME_MS) {
    motorOn = false;
    analogWrite(MOTOR_1_PIN, 0);
  }

  if (playClick) {
    sdSource->close();
    if (sdSource->open("click.wav")) 
      wav->begin(sdSource, out);
    playClick = false;
  }
  
  if (wav->isRunning())
    if (!wav->loop())
      wav->stop();

  IrSender.sendNEC(0x00, 0x34 + distance, 0);
}

void setup1() {
	pixel.begin();
  pixel.clear();
  pixel.setPixelColor(0, pixel.Color(0, 255, 0));
  pixel.show();
}

void loop1() {
  static int brightness = 0;
  static int fadeDir = 1;

  pixel.setPixelColor(0, pixel.Color(
    (red * b) / 255,
    (green * b) / 255,
    (blue * b) / 255
  ));
  pixel.show();

  brightness += fadeDir;
  if (fadeDir > 0 && brightness == 255) {
    fadeDir = -1;
    playClick = true;
    motorOn = true;
    motorStartTime = millis();
    analogWrite(MOTOR_1_PIN, 255);
  } else if (fadeDir < 0 && brightness == 0) {
    fadeDir = 1;
  }

  delay(stepDelay);
}

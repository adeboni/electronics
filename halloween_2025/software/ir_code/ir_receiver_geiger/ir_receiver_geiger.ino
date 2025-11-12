#define NEOPIXEL_PIN   7
#define IR_RECEIVE_PIN 8
#define IR_SEND_PIN    9
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
#define DECODE_NEC
#include <IRremote.hpp>
#include <SPI.h>
#include <SD.h>
#include <AudioFileSourceSD.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2S.h>
#include <Adafruit_NeoPixel.h>

File file;
AudioFileSourceSD *sdSource;
AudioGeneratorWAV *wav;
AudioOutputI2S *out;
bool playClick = false;

Adafruit_NeoPixel pixel(1, NEOPIXEL_PIN, NEO_RGB + NEO_KHZ800);

float frequency = 1.0;
float cycleTime = 1.0;
float stepDelay = 10.0;

// Color to pulse
uint8_t red = 52;
uint8_t green = 235;
uint8_t blue = 88;

void setup() {
  pinMode(INPUT_VOLT_PIN, INPUT);

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

  IrReceiver.begin(IR_RECEIVE_PIN, false);
}

void loop() {
  checkIR();

  if (playClick) {
    sdSource->close();
    if (getDip(0) && sdSource->open("click.wav")) 
      wav->begin(sdSource, out);
    playClick = false;
  }

  if (wav->isRunning())
    if (!wav->loop())
      wav->stop();
}

void checkIR() {
  if (IrReceiver.decode()) {
    if ((IrReceiver.decodedIRData.decodedRawData & 0xFFFF) == 0xFF00) {
      int distance = ((IrReceiver.decodedIRData.decodedRawData >> 16) & 0xFF) - 0x34;
      frequency = (200 - distance) / 10.0;
      cycleTime = 1.0 / frequency;
      stepDelay = (cycleTime / 512) * 1000.0;
    }
    IrReceiver.resume();
  }
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
  } else if (fadeDir < 0 && brightness == 0) {
    fadeDir = 1;
  }

  delay(stepDelay);
}

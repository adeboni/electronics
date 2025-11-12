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

uint8_t dipPins[6] = {0, 1, 2, 3, 4, 5};
uint8_t relayPins[4] = {22, 23, 24, 25};
int relayOnTimesMs[4] = {100, 500, 1000, 5000};
unsigned long relayStartTime[4] = {0, 0, 0, 0};
bool relayState[4] = {false, false, false, false};

File file;
AudioFileSourceSD *sdSource;
AudioGeneratorWAV *wav;
AudioOutputI2S *out;

void setup() {
  pinMode(INPUT_VOLT_PIN, INPUT);
  for (int i = 0; i < 6; i++)
    pinMode(dipPins[i], INPUT_PULLUP);
  for (int i = 0; i < 4; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);
  }

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

  for (int i = 0; i < 4; i++) {
    if (!relayState[i]) continue;
    if (millis() - relayStartTime[i] > relayOnTimesMs[i]) {
      digitalWrite(relayPins[i], LOW);
      relayState[i] = false;
    }
  }

  if (wav->isRunning())
    if (!wav->loop())
      wav->stop();
}

void checkIR() {
  if (IrReceiver.decode()) {
    if ((IrReceiver.decodedIRData.decodedRawData & 0xFFFF) == 0xFF00) {
      int blasterID = ((IrReceiver.decodedIRData.decodedRawData >> 16) & 0xFF) - 0x34;
      Serial.print("Got hit by ID ");
      Serial.println(blasterID);

      sdSource->close();
      if (getDip(0) && sdSource->open("explosion.wav")) 
        wav->begin(sdSource, out);

      for (int i = 0; i < 4; i++) {
        if (!getDip(2 + i)) continue;
        digitalWrite(relayPins[i], HIGH);
        relayState[i] = true;
        relayStartTime[i] = millis();
      }
    }
    IrReceiver.resume();
  }
}

bool getDip(int index) {
  return digitalRead(dipPins[index]) == LOW;
}
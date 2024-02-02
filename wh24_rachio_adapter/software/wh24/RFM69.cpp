#include "RFM69.h"
#include <SPI.h>

uint8_t RFM69::DATA[60];
uint8_t RFM69::_mode;
volatile bool RFM69::_haveData;

bool RFM69::initialize() {
  _mode = RF69_MODE_STANDBY;
  _spi = &SPI;

  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  uint8_t interruptNum = digitalPinToInterrupt(RFM69_INT);

  digitalWrite(RFM69_CS, HIGH);
  pinMode(RFM69_CS, OUTPUT);
  _spi->begin();
  _settings = SPISettings(8000000, MSBFIRST, SPI_MODE0);

  uint32_t start = millis();
  uint8_t timeout = 50;
  do writeReg(0x2F, 0xAA); while (readReg(0x2F) != 0xAA && millis() - start < timeout);
  if (millis() - start >= timeout) return false;
  start = millis();
  do writeReg(0x2F, 0x55); while (readReg(0x2F) != 0x55 && millis() - start < timeout);
  if (millis() - start >= timeout) return false;

  writeReg(0x01, 0x04);
  writeReg(0x02, 0x00);
  writeReg(0x03, 0x07);
  writeReg(0x04, 0x40);
  writeReg(0x05, 0x01);
  writeReg(0x06, 0x9A);
  writeReg(0x07, 0xE4);
  writeReg(0x08, 0xC0);
  writeReg(0x09, 0x00);
  writeReg(0x19, 0x2B);
  writeReg(0x25, 0x40);
  writeReg(0x26, 0x07);
  writeReg(0x28, 0x10);
  writeReg(0x29, 0xE4);
  writeReg(0x2D, 0x03);
  writeReg(0x2E, 0x80 | 0x08);
  writeReg(0x2F, 0x2D);
  writeReg(0x30, 0xD4);
  writeReg(0x37, 0x80);
  writeReg(0x38, 66);
  writeReg(0x3C, 0x80 | 0x0F);
  writeReg(0x3D, 0x02);
  writeReg(0x6F, 0x30);
  writeReg(0x13, 0x0F);
  writeReg(0x5A, 0x55);
  writeReg(0x5C, 0x70);
  writeReg(0x11, 0x40 | 0x20 | 31);

  start = millis();
  while (((readReg(0x27) & 0x80) == 0x00) && millis() - start < timeout); // wait for ModeReady
  if (millis() - start >= timeout) return false;
  attachInterrupt(interruptNum, RFM69::isr0, RISING);

  return true;
}

void RFM69::setMode(uint8_t newMode) {
  if (newMode == _mode) return;
  writeReg(0x01, (readReg(0x01) & 0xE3) | (newMode == RF69_MODE_RX ? 0x10 : 0x04));
  _mode = newMode;
}

void RFM69::interruptHandler() {
  if (_mode == RF69_MODE_RX && (readReg(0x28) & 0x04)) {
    setMode(RF69_MODE_STANDBY);
    select();
    _spi->transfer(0);
    for (uint8_t i = 0; i < 60; i++) DATA[i] = _spi->transfer(0);
    unselect();
    setMode(RF69_MODE_RX);
  }
}

void RFM69::isr0() {
  _haveData = true;
}

void RFM69::receiveBegin() {
  if (readReg(0x28) & 0x04)
    writeReg(0x3D, (readReg(0x3D) & 0xFB) | 0x04);
  writeReg(0x25, 0x40);
  setMode(RF69_MODE_RX);
}

bool RFM69::receiveDone() {
  bool havePayload = false;
  if (_haveData) {
  	_haveData = false;
  	interruptHandler();
    havePayload = true;
  }
  if (_mode == RF69_MODE_RX && havePayload) {
    setMode(RF69_MODE_STANDBY);
    return true;
  } else if (_mode == RF69_MODE_RX) { 
    return false;
  }
  receiveBegin();
  return false;
}

uint8_t RFM69::readReg(uint8_t addr) {
  select();
  _spi->transfer(addr & 0x7F);
  uint8_t regval = _spi->transfer(0);
  unselect();
  return regval;
}

void RFM69::writeReg(uint8_t addr, uint8_t value) {
  select();
  _spi->transfer(addr | 0x80);
  _spi->transfer(value);
  unselect();
}

void RFM69::select() {
  _spi->beginTransaction(_settings);
  digitalWrite(RFM69_CS, LOW);
}

void RFM69::unselect() {
  digitalWrite(RFM69_CS, HIGH);
  _spi->endTransaction();
}

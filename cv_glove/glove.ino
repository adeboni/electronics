#include <Wire.h>
#include <Adafruit_MCP4725.h>

Adafruit_MCP4725 dac1;
Adafruit_MCP4725 dac2;
Adafruit_MCP4725 dac3;

const int DAC1_ENABLE = A0;
const int DAC2_ENABLE = A2;
const int DAC3_ENABLE = A7;

const int PIN_THUMB = A3;
const int PIN_POINTER = A6;
const int PIN_RING = A1;

const float VCC = 3.32;
const float R_DIV = 47000.0; 

const float STRAIGHT_RESISTANCE = 28500.0;
const float BEND_RESISTANCE = 90000.0;

void setup() 
{
  //Serial.begin(9600);
  pinMode(PIN_THUMB, INPUT);
  pinMode(PIN_POINTER, INPUT);
  pinMode(PIN_RING, INPUT);

  pinMode(DAC1_ENABLE, OUTPUT);
  pinMode(DAC2_ENABLE, OUTPUT);
  pinMode(DAC3_ENABLE, OUTPUT);

  digitalWrite(DAC1_ENABLE, LOW);
  digitalWrite(DAC2_ENABLE, LOW);
  digitalWrite(DAC3_ENABLE, LOW);

  pinMode(11, OUTPUT);
  digitalWrite(11, LOW);

  dac1.begin(0x63);
  dac2.begin(0x63);
  dac3.begin(0x63);
}

void loop() 
{
  //Serial.println(String(angle(PIN_THUMB)) + " " + String(angle(PIN_POINTER)) + " " + String(angle(PIN_RING)));
  //Serial.println(String(mappedAngle(PIN_THUMB)) + " " + String(mappedAngle(PIN_POINTER)) + " " + String(mappedAngle(PIN_RING)));
  dacWrite(mappedAngle(PIN_POINTER), mappedAngle(PIN_THUMB), mappedAngle(PIN_RING));
  delay(100);
}

double angle(int pin) 
{
  int flexADC = analogRead(pin);
  float flexV = flexADC * VCC / 1023.0;
  float flexR = R_DIV * (VCC / flexV - 1.0);
  return map(flexR, STRAIGHT_RESISTANCE, BEND_RESISTANCE, 0, 90.0);
}

uint32_t mappedAngle(int pin) 
{
  int flexADC = analogRead(pin);
  float flexV = flexADC * VCC / 1023.0;
  float flexR = R_DIV * (VCC / flexV - 1.0);
  return constrain(map(flexR, STRAIGHT_RESISTANCE, BEND_RESISTANCE, 0, 4095), 0, 4095);
}

void dacWrite(uint32_t dac1v, uint32_t dac2v, uint32_t dac3v) 
{
  digitalWrite(DAC1_ENABLE, HIGH);
  digitalWrite(DAC2_ENABLE, LOW);
  digitalWrite(DAC3_ENABLE, LOW);
  dac1.setVoltage(dac1v, false);

  digitalWrite(DAC1_ENABLE, LOW);
  digitalWrite(DAC2_ENABLE, HIGH);
  digitalWrite(DAC3_ENABLE, LOW);
  dac2.setVoltage(dac2v, false);

  digitalWrite(DAC1_ENABLE, LOW);
  digitalWrite(DAC2_ENABLE, LOW);
  digitalWrite(DAC3_ENABLE, HIGH);
  dac3.setVoltage(dac3v, false);
}

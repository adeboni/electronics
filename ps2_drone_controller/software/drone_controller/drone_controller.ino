#include <Ps3Controller.h>
#include <SPI.h>

#define MIN_DIFF 40

SPIClass * vspi = NULL;

int lastDacVals[6] = {0, 0, 0, 0, 0, 0};
int dacVals[6] = {2048, 2048, 2048, 2048, 4095, 4095};
int dacPins[6] = {4, 0, 17, 16, 2, 15};

void setup() {
  for (int i = 0; i < 6; i++) {
    pinMode(dacPins[i], OUTPUT);
    digitalWrite(dacPins[i], HIGH);
  }
  vspi = new SPIClass(VSPI);
  vspi->begin();
  writeDACs();

  Serial.begin(9600);
  Ps3.begin("01:02:03:04:05:06");
  while (!Ps3.isConnected()) {
    Serial.println("Waiting for connection...");
    delay(500);
  }
    
  Serial.println("Connected");
  Ps3.setPlayer(2);
  delay(500);

  Serial.println("Checking battery status");
  int battery = Ps3.data.status.battery;
  Serial.printf("The controller battery is %d = ", battery);
  if (battery == ps3_status_battery_charging)      Serial.println("charging");
  else if (battery == ps3_status_battery_full)     Serial.println("FULL");
  else if (battery == ps3_status_battery_high)     Serial.println("HIGH");
  else if (battery == ps3_status_battery_low)      Serial.println("LOW");
  else if (battery == ps3_status_battery_dying)    Serial.println("DYING");
  else if (battery == ps3_status_battery_shutdown) Serial.println("SHUTDOWN");
  else Serial.println("UNDEFINED");
}

void loop() { 
  int vals[4] = {Ps3.data.analog.stick.lx, -Ps3.data.analog.stick.ly, Ps3.data.analog.stick.rx, -Ps3.data.analog.stick.ry};
  for (int i = 0; i < 4; i++)
    dacVals[i] = map(abs(vals[i]) < 12 ? 0 : vals[i], -128, 128, 0, 4095);

  if (Ps3.data.button.start) { //takeoff
    dacVals[4] = 674;
    dacVals[5] = 4095;
  } else if (Ps3.data.button.select) { //headless
    dacVals[4] = 941;
    dacVals[5] = 4095;
  } else if (Ps3.data.button.l3) { //calibrate
    dacVals[4] = 367;
    dacVals[5] = 4095;
  } else if (Ps3.data.button.up) { //flip
    dacVals[4] = 1369;
    dacVals[5] = 4095;
  } else if (Ps3.data.button.l1) { //speed
    dacVals[4] = 4095;
    dacVals[5] = 0;
  } else {
    dacVals[4] = 4095;
    dacVals[5] = 4095;
  }

  writeDACs();
} 

void writeDACs() {
  for (int i = 0; i < 6; i++) {
    //Serial.printf("%d\t", dacVals[i]);
    writeDAC(i);
  }
  //Serial.println();
}

void writeDAC(int i) {
  if (abs(lastDacVals[i] - dacVals[i]) < MIN_DIFF)
    return;
  
  lastDacVals[i] = dacVals[i];
  digitalWrite(dacPins[i], LOW);
  vspi->transfer(B00010001);
  vspi->transfer(dacVals[i] >> 4);
  digitalWrite(dacPins[i], HIGH);
}

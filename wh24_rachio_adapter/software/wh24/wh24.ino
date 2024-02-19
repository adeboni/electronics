#include "cppQueue.h"
#include "RFM69.h"

#define DAY_MS    86400000UL
#define SET_PIN   24
#define UNSET_PIN 25
#define RELAY_LED 5

typedef struct {
  float level;
  unsigned long time;
} rain_t;

RFM69 radio;
cppQueue queue(sizeof(rain_t), 6000, FIFO);
float rain_level = -1;
unsigned long water_stop_end = 0;
bool water_on = false;
int num_items = 0;

void setup() { 
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_LED, OUTPUT);
  pinMode(SET_PIN, OUTPUT);
  pinMode(UNSET_PIN, OUTPUT);

  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(RELAY_LED, LOW);
  digitalWrite(SET_PIN, LOW);
  digitalWrite(UNSET_PIN, LOW);

  turn_water_on();

  Serial.begin(115200);
  //while (!Serial) delay(1);
  radio.initialize();
  Serial.println("Radio started");
}

uint8_t crc8() {
  uint8_t crc = 0;
  for (int i = 0; i < 15; i++) {
    crc ^= radio.DATA[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 0x80)
        crc = (crc << 1) ^ 0x31;
      else
        crc = (crc << 1);
    }
  }
  return crc;
}

uint8_t checksum() {
  uint8_t checksum = 0;
  for (int i = 0; i < 16; i++)
    checksum += radio.DATA[i];
  return checksum;
}

bool decode() {
  if (radio.DATA[0] != 0x24 || radio.DATA[15] != crc8() || radio.DATA[16] != checksum()) 
    return false;
  
  int id              = radio.DATA[1];                                               // changes on battery change
  int wind_dir        = radio.DATA[2] | (radio.DATA[3] & 0x80) << 1;                 // range 0-359 deg, 0x1ff if invalid
  int low_battery     = (radio.DATA[3] & 0x08) >> 3;
  int temp_raw        = (radio.DATA[3] & 0x07) << 8 | radio.DATA[4];                 // 0x7ff if invalid
  float temperature   = (temp_raw - 400) * 0.1f;                                     // range -40.0-60.0 C
  int humidity        = radio.DATA[5];                                               // 0xff if invalid
  int wind_speed_raw  = radio.DATA[6] | (radio.DATA[3] & 0x10) << 4;                 // 0x1ff if invalid
  float wind_speed_ms = wind_speed_raw * 0.125f * 1.12f;
  int gust_speed_raw  = radio.DATA[7];                                               // 0xff if invalid
  float gust_speed_ms = gust_speed_raw * 1.12f;
  int rainfall_raw    = radio.DATA[8] << 8 | radio.DATA[9];                          // rain tip counter
  float rainfall_mm   = rainfall_raw * 0.3f;                                         // each tip is 0.3mm / 0.254mm
  int uv_raw          = radio.DATA[10] << 8 | radio.DATA[11];                        // range 0-20000, 0xffff if invalid
  int light_raw       = radio.DATA[12] << 16 | radio.DATA[13] << 8 | radio.DATA[14]; // 0xffffff if invalid
  double light_lux    = light_raw * 0.1;                                             // range 0.0-300000.0lux

  Serial.println("-----------------------------");
  Serial.print("id:             "); Serial.println(id);
  Serial.print("wind_dir:       "); Serial.println(wind_dir);
  Serial.print("low_battery:    "); Serial.println(low_battery);
  Serial.print("temp_raw:       "); Serial.println(temp_raw);
  Serial.print("temperature:    "); Serial.println(temperature);
  Serial.print("humidity:       "); Serial.println(humidity);
  Serial.print("wind_speed_raw: "); Serial.println(wind_speed_raw);
  Serial.print("wind_speed_ms:  "); Serial.println(wind_speed_ms);
  Serial.print("gust_speed_raw: "); Serial.println(gust_speed_raw);
  Serial.print("gust_speed_ms:  "); Serial.println(gust_speed_ms);
  Serial.print("rainfall_raw:   "); Serial.println(rainfall_raw);
  Serial.print("rainfall_mm:    "); Serial.println(rainfall_mm);
  Serial.print("uv_raw:         "); Serial.println(uv_raw);
  Serial.print("light_raw:      "); Serial.println(light_raw);
  Serial.print("light_lux:      "); Serial.println(light_lux);
  Serial.print("crc:            "); Serial.println(radio.DATA[15]);
  Serial.print("checksum:       "); Serial.println(radio.DATA[16]);
  Serial.println("-----------------------------");

  rain_level = rainfall_mm;
  return true;
}

void updateOutputs() {
  unsigned long curr_time = millis();
  rain_t new_measurement = {rain_level, curr_time};
  queue.push(&new_measurement);
  num_items++;

  rain_t r;
  queue.peek(&r);
  while (num_items > 1 && curr_time - r.time > DAY_MS) {
    queue.pop(&r);
    queue.peek(&r);
    num_items--;
  }

  //stop watering for 4 days if it rained over 12 mm in at least 12 hours
  if (curr_time - r.time > DAY_MS / 2UL && rain_level - r.level > 12) { 
    water_stop_end = curr_time + DAY_MS * 4UL;
  } else {
    for (int i = 0; i < queue.getCount(); i++) {
      queue.peekIdx(&r, i);
      if (curr_time - r.time < DAY_MS / 24UL) {
        //stop watering for 3 days if it rained over an 3 mm in 1 hour
        if (rain_level - r.level > 3) 
          water_stop_end = curr_time + DAY_MS * 3UL;
        break;
      }
    }
  }

  if (water_stop_end > curr_time) 
	  turn_water_off();
  else
	  turn_water_on();
}

void loop() {
  if (radio.receiveDone()) {
    if (decode()) {
      blink(LED_BUILTIN, 40, 3);
      updateOutputs();
    }
  }
}

void blink(int pin, int delay_ms, int loops) {
  while (loops--) {
    digitalWrite(pin, LOW);
    delay(delay_ms);
    digitalWrite(pin, HIGH);
    delay(delay_ms);
  }
}

void turn_water_off() {
  if (!water_on) return;
  Serial.println("Turning water off");
  water_on = false;
  digitalWrite(RELAY_LED, HIGH);
  digitalWrite(UNSET_PIN, HIGH);
  delay(500);
  digitalWrite(UNSET_PIN, LOW);
}

void turn_water_on() {
  if (water_on) return;
  Serial.println("Turning water on");
  water_on = true;
  digitalWrite(RELAY_LED, LOW);
  digitalWrite(SET_PIN, HIGH);
  delay(500);
  digitalWrite(SET_PIN, LOW);
}

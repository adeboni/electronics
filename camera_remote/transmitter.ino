#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(2, 3); // CE, CSN         
const byte address[6] = "00001";
int button_pin = 1;
boolean button_state = 0;
int led_pin = 0;

void setup() {
  pinMode(led_pin, OUTPUT);

  pinMode(button_pin, INPUT_PULLUP);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  digitalWrite(led_pin, HIGH);
  delay(100);
  digitalWrite(led_pin, LOW);
  delay(100);
}

void loop()
{
  button_state = !digitalRead(button_pin);
  radio.write(&button_state, sizeof(button_state));
  delay(20);
}

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(2, 3); // CE, CSN
const byte address[6] = "00001";
boolean button_state = 0;
int led_pin = 1;

void setup() {
  pinMode(led_pin, OUTPUT);
  
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  digitalWrite(led_pin, HIGH);
  delay(100);
  digitalWrite(led_pin, LOW);
  delay(100);
}

void loop()
{
  
  if (radio.available())
  {
    radio.read(&button_state, sizeof(button_state));
    digitalWrite(led_pin, button_state);
  }
  delay(5);
}

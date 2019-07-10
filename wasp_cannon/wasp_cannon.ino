#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <Servo.h>

Servo pan;
Servo tilt;

int panPos = 90; 
int panMin = 0;
int panMax = 180;
int tiltPos = 0; 
int tiltMin = 0;
int tiltMax = 100;

int status = WL_IDLE_STATUS;
#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

unsigned int localPort = 2390;      // local port to listen on

char packetBuffer[255]; //buffer to hold incoming packet
char  ReplyBuffer[] = "acknowledged";       // a string to send back

WiFiUDP Udp;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, OUTPUT);
  pan.attach(5);
  tilt.attach(4);
  pan.write(panPos);
  tilt.write(tiltPos);
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) { }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    //Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    //Serial.print("Attempting to connect to SSID: ");
    //Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  Serial.println("Connected to wifi");
  printWifiStatus();

  Udp.begin(localPort);
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    
    // read the packet into packetBufffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
	int xval = getValue(String(packetBuffer), '/', 0).toInt();
	int yval = getValue(String(packetBuffer), '/', 1).toInt();
	int tval = getValue(String(packetBuffer), '/', 2).toInt();
	Serial.print(xval);
	Serial.print(" ");
	Serial.print(yval);
	Serial.print(" ");
	Serial.println(tval);
     
    panPos = max(panMin, min(panMax, panPos + xval));
    tiltPos = max(tiltMin, min(tiltMax, tiltPos + yval));

	Serial.print(panPos);
	Serial.print(" ");
	Serial.println(tiltPos);
     
    pan.write(panPos);
    tilt.write(tiltPos);
    analogWrite(LED_BUILTIN, tval);
    digitalWite(0, tval == 255 ? 1 : 0);
  }


}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

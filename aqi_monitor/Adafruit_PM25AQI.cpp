#include "Adafruit_PM25AQI.h"

/*!
 *  @brief  Instantiates a new PM25AQI class
 */
Adafruit_PM25AQI::Adafruit_PM25AQI() {}

/*!
 *  @brief  Setups the hardware and detects a valid UART PM2.5
 *  @param  theSerial
 *          Pointer to Stream (HardwareSerial/SoftwareSerial) interface
 *  @return True
 */
bool Adafruit_PM25AQI::begin_UART(Stream *theSerial) {
  serial_dev = theSerial;
  //TODO: autosend https://forum.arduino.cc/index.php?topic=471975.15
  byte start_autosend[] = {0x68, 0x01, 0x40, 0x57 };
  serial_dev->write(start_autosend, sizeof(start_autosend));
  return true;
}

/*!
 *  @brief  Setups the hardware and detects a valid UART PM2.5
 *  @param  data
 *          Pointer to PM25_AQI_Data that will be filled by read()ing
 *  @return True on successful read, false if timed out or bad data
 */
bool Adafruit_PM25AQI::read(PM25_AQI_Data *data) {
  uint8_t buffer[32];
  uint16_t sum = 0;

  if (!data) {
    return false;
  }

  if (serial_dev) { // ok using uart
    if (!serial_dev->available()) {
      return false;
    }
    if (serial_dev->peek() != 0x42) {
      serial_dev->read();
      return false;
    }
    // Now read all 32 bytes
    if (serial_dev->available() < 32) {
      return false;
    }
    serial_dev->readBytes(buffer, 32);
  } else {
    return false;
  }

  // Check that start byte is correct!
  if (buffer[0] != 0x42) {
    return false;
  }

  // get checksum ready
  for (uint8_t i = 0; i < 30; i++) {
    sum += buffer[i];
  }

  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i = 0; i < 15; i++) {
    buffer_u16[i] = buffer[2 + i * 2 + 1];
    buffer_u16[i] += (buffer[2 + i * 2] << 8);
  }

  // put it into a nice struct :)
  memcpy((void *)data, (void *)buffer_u16, 30);

  if (sum != data->checksum) {
    return false;
  }

  // success!
  return true;
}

#include "Arduino.h"

typedef struct PMSAQIdata {
  uint16_t framelen;       ///< How long this data chunk is
  uint16_t pm10_standard,  ///< Standard PM1.0
      pm25_standard,       ///< Standard PM2.5
	  pm40_standard,       ///< Standard PM2.5
      pm100_standard;      ///< Standard PM10.0
  uint16_t reserved0,       ///< Reserved
      reserved1,            ///< Reserved
	  reserved2,            ///< Reserved
	  reserved3,            ///< Reserved
	  reserved4,            ///< Reserved
	  reserved5,            ///< Reserved
	  reserved6,            ///< Reserved
	  reserved7,            ///< Reserved
	  reserved8;            ///< Reserved
  uint16_t checksum;       ///< Packet checksum
} PM25_AQI_Data;

/*!
 *  @brief  Class that stores state and functions for interacting with
 *          PM2.5 Air Quality Sensor
 */
class Adafruit_PM25AQI {
public:
  Adafruit_PM25AQI();
  bool begin_UART(Stream *theStream);
  bool read(PM25_AQI_Data *data);

private:
  Stream *serial_dev = NULL;
  uint8_t _readbuffer[32];
};

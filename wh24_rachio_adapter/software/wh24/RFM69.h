#ifndef RFM69_h
#define RFM69_h
#include <SPI.h>

#define RF69_IRQ_PIN      2
#define RF69_MODE_STANDBY 0
#define RF69_MODE_RX      1
#define RFM69_CS          16
#define RFM69_INT         21
#define RFM69_RST         17

class RFM69 {
  public:
    static uint8_t DATA[60];
    static uint8_t _mode;

    bool initialize();
    virtual bool receiveDone();
    uint8_t readReg(uint8_t addr);
    void writeReg(uint8_t addr, uint8_t val);
    virtual void setMode(uint8_t mode);
    virtual void select();
    virtual void unselect();

  protected:
    static void isr0();
    void interruptHandler();
    static volatile bool _haveData;
    SPIClass *_spi;
    SPISettings _settings;
    virtual void receiveBegin();
};

#endif

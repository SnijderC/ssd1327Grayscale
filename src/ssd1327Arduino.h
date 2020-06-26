#include "ssd1327.h"
#include <Wire.h>
#include <SPI.h>

#ifndef DEFAULT_SPI_SPEED
#define DEFAULT_SPI_SPEED 4000000L
#endif

namespace Ssd1327 {

class ArduinoI2cInterface: public Interface {
  public:
    TwoWire* _i2c;
    uint8_t _address;
    void beginTransmission();
    uint8_t endTransmission();
    void write(uint8_t byte);
    uint8_t sendCommand(uint8_t command);
    uint8_t sendCommand(uint8_t* command, uint8_t len);
    uint8_t sendData(uint8_t* data, uint16_t len);
    ArduinoI2cInterface(TwoWire* i2c, uint8_t i2cAddress);
    void begin();
};

class ArduinoSpiInterface: public Interface {
  private:
    SPIClass _spi;
    int8_t _dc;
    int8_t _cs;
    int8_t _rst;
    unsigned long _spiSpeed;
    SPISettings _spiSettings;
  public:
    void beginTransmission();
    uint8_t endTransmission();
    void write(uint8_t byte);
    uint8_t sendCommand(uint8_t command);
    uint8_t sendCommand(uint8_t* command, uint8_t len);
    uint8_t sendData(uint8_t* data, uint16_t len);
    bool hWreset();
    ArduinoSpiInterface(
      SPIClass spi,
      int8_t dc,
      int8_t cs,
      int8_t rst,
      unsigned long spiSpeed
    );
    void begin();
};

class ArduinoImplementation: public Implementation {
  private:
    void _waitms(uint16_t ms);
  public:
    ArduinoImplementation(
      uint8_t width, uint8_t height, TwoWire* i2c, uint8_t i2cAddress
    );
    ArduinoImplementation(
      uint8_t width,
      uint8_t height,
      SPIClass spi,
      int8_t dc,
      int8_t cs,
      int8_t rst,
      unsigned long spiSpeed
    );
    ArduinoImplementation(
      uint8_t width,
      uint8_t height,
      SPIClass spi,
      int8_t dc,
      int8_t cs,
      int8_t rst
    );
    ArduinoImplementation(
      uint8_t width,
      uint8_t height,
      SPIClass spi,
      int8_t dc,
      int8_t cs
    );
    ArduinoImplementation(
      uint8_t width,
      uint8_t height,
      SPIClass spi,
      int8_t dc
    );
    uint8_t begin();
};
}

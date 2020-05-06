#include "ssd1327.h"
#include <Wire.h>

class Ssd1327Arduino: public Ssd1327 {
  private:
    TwoWire* _i2c;
    uint8_t _address;
    void _beginTransmission();
    void _write(uint8_t data);
    uint8_t _endTransmission();
    void _waitms(uint16_t ms);
  public:
    Ssd1327Arduino(TwoWire* i2c, uint8_t i2cAddress, uint8_t width, uint8_t height);
    uint8_t begin();
};

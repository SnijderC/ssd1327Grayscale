#include "ssd1327Arduino.h"
#include <Arduino.h>

namespace Ssd1327 {

uint8_t ArduinoI2cInterface::sendCommand(uint8_t *command, uint8_t len)
{
  _i2c->beginTransmission(_address);
  _i2c->write(0x00); // Control byte 0x00 specifies commands.
  while(len--)
  {
    _i2c->write(*command++);
  }
  return _i2c->endTransmission(true);
}

uint8_t ArduinoI2cInterface::sendCommand(uint8_t command)
{
  _i2c->beginTransmission(_address);
  _i2c->write(0x00); // Control byte 0x00 specifies commands.
  _i2c->write((uint8_t)command);
  return _i2c->endTransmission(true);
}

uint8_t ArduinoI2cInterface::sendData(uint8_t *data, uint16_t len)
{
  _i2c->beginTransmission(_address);
  _i2c->write(0x40); // Control byte 0x40 specifies data.
  uint8_t error = 0;
  // Send data in chunks that fit in the display module I2C buffer.
  while (len--)
  {
    // Reset connection for every buffer length minus address and control byte
    if (len % (SSD1327_MAX_I2C_BUFFER - 2) == 0)
    {
      error = _i2c->endTransmission(true);
      if (error !=0) return error;
      _i2c->beginTransmission(_address);
      _i2c->write(0x40);
    }
    _i2c->write(*data++);
  }
  return _i2c->endTransmission(true);
}

void ArduinoI2cInterface::beginTransmission()
{
  _i2c->beginTransmission(_address);
}
uint8_t ArduinoI2cInterface::endTransmission()
{
  return _i2c->endTransmission(true);
}
void ArduinoI2cInterface::write(uint8_t byte)
{
  _i2c->write(byte);
}

ArduinoI2cInterface::ArduinoI2cInterface(
  TwoWire* i2c, uint8_t i2cAddress
) {
  type = (uint8_t)Interface::InterfaceType::I2c;
}
void ArduinoI2cInterface::begin()
{
  _i2c->begin();
}
uint8_t ArduinoSpiInterface::sendCommand(uint8_t *command, uint8_t len)
{
  beginTransmission();
  if (_dc > Interface::NO_PIN)
  {
    // Serial.println("CMD: Setting D/C# low.");
    digitalWrite(_dc, LOW);
  }
  // for (uint8_t i = 0; i < len; i++) 
  // {
  //   Serial.printf("CMD: 0x%02x\n", command[i]);
  // }
  _spi.transfer(command, len);
  
  if (_dc > Interface::NO_PIN)
  {
    // Serial.println("CMD: Setting D/C# high.");
    digitalWrite(_dc, HIGH);
  }
  return endTransmission();
}

uint8_t ArduinoSpiInterface::sendCommand(uint8_t command)
{
  beginTransmission();
  if (_dc > Interface::NO_PIN)
  {
    // Serial.println("CMD: Setting D/C# low.");
    digitalWrite(_dc, LOW);
  }
  // Serial.printf("CMD: 0x%02x\n", command);
  _spi.transfer(command);
  if (_dc > Interface::NO_PIN)
  {
    // Serial.println("CMD: Setting D/C# high.");
    digitalWrite(_dc, HIGH);
  }
  return endTransmission();
}

uint8_t ArduinoSpiInterface::sendData(uint8_t *data, uint16_t len)
{
  if (_dc > Interface::NO_PIN)
  {
    // Serial.println("DATA: Setting D/C# high.");
    digitalWrite(_dc, HIGH);
  }
  beginTransmission();
  // for (uint16_t i = 0; i < len; i++) 
  // {
  //   // Serial.printf("DATA: 0x%02x\n", data[i]);
  // }
  // Send data in chunks that fit in the display module SPI buffer.
  while (len--)
  {
    // Reset connection for every buffer length
    if (len % SSD1327_MAX_SPI_BUFFER == 0)
    {
      endTransmission();
      beginTransmission();
    }
    _spi.transfer(*data++);
  }
  return endTransmission();
}

void ArduinoSpiInterface::beginTransmission() {
  if (_cs > Interface::NO_PIN)
  {
    // Serial.println("BEGIN: Setting CS# low.");
    digitalWrite(_cs, LOW);
  }
}
void ArduinoSpiInterface::write(uint8_t data)
{
  _spi.transfer(data);
}
uint8_t ArduinoSpiInterface::endTransmission()
{
  if (_cs > Interface::NO_PIN)
  {
    // Serial.println("END: Setting CS# high.");
    digitalWrite(_cs, HIGH);
  }
  return 0;
}

bool ArduinoSpiInterface::hWreset()
{
  if (_rst == Interface::NO_PIN) return false;
  // Serial.println("Hardware reset!");
	digitalWrite(_rst, LOW);
	delay(100);
	digitalWrite(_rst, HIGH);
	delay(100);
  return true;
}

ArduinoSpiInterface::ArduinoSpiInterface(
    SPIClass spi,
    int8_t dc,
    int8_t cs,
    int8_t rst,
    unsigned long spiSpeed
): _spi(spi), _dc(dc), _cs(cs), _rst(rst)
{
  type = (uint8_t)Interface::InterfaceType::Spi;
  _spiSettings = SPISettings(spiSpeed, MSBFIRST, SPI_MODE0); // sure
}
void ArduinoSpiInterface::begin() {
  _spi.beginTransaction(_spiSettings);
  if (_rst > Interface::NO_PIN) {
    // Serial.println("INIT: Setting RST# low.");
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW);
    delay(100);
    // Serial.println("INIT: Setting RST# high.");
    digitalWrite(_rst, HIGH);
  }
  if (_dc > Interface::NO_PIN)
  {
    // Serial.println("INIT: Setting D/C# high.");
    pinMode(_dc, OUTPUT);
    digitalWrite(_dc, HIGH);
  }
  if (_cs > Interface::NO_PIN)
  {
    // Serial.println("INIT: Setting CS# high.");
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH);
  }
}

ArduinoImplementation::ArduinoImplementation(
  uint8_t width,
  uint8_t height,
  SPIClass spi,
  int8_t dc,
  int8_t cs,
  int8_t rst,
  unsigned long spiSpeed
): Implementation(width, height)
{
  interface = new ArduinoSpiInterface(spi, dc, cs, rst, spiSpeed);
}
ArduinoImplementation::ArduinoImplementation(
  uint8_t width,
  uint8_t height,
  SPIClass spi,
  int8_t dc,
  int8_t cs,
  int8_t rst
): ArduinoImplementation(width, height, spi, dc, cs, rst, DEFAULT_SPI_SPEED)
{}
ArduinoImplementation::ArduinoImplementation(
  uint8_t width,
  uint8_t height,
  SPIClass spi,
  int8_t dc,
  int8_t cs
): ArduinoImplementation(
    width,
    height,
    spi,
    dc,
    cs,
    Interface::NO_PIN,
    DEFAULT_SPI_SPEED
  )
{}
ArduinoImplementation::ArduinoImplementation(
  uint8_t width,
  uint8_t height,
  SPIClass spi,
  int8_t dc
): ArduinoImplementation(
    width,
    height,
    spi,
    dc,
    Interface::NO_PIN,
    Interface::NO_PIN,
    DEFAULT_SPI_SPEED
  )
{}

ArduinoImplementation::ArduinoImplementation(
  uint8_t width, uint8_t height, TwoWire* i2c, uint8_t i2cAddress
):
  Implementation(width, height)
{
  interface = new ArduinoI2cInterface(i2c, i2cAddress);
}
// This needs to be implemented to reclaim memory if the display object is
// deleted.
// ArduinoImplementation::~ArduinoImplementation() {
//   switch (interface->type) {
//     case (uint8_t)Interface::InterfaceType::Spi:
//       //ArduinoSpiInterface *spiPtr = (ArduinoSpiInterface*)interface;
//       delete(((ArduinoSpiInterface*)interface));
//       break;
//     case (uint8_t)Interface::InterfaceType::I2c:
//       //ArduinoI2cInterface *ic2Ptr = (ArduinoI2cInterface*)interface;
//       //delete(ic2Ptr);
//       delete(((ArduinoI2cInterface*)interface));
//       break;
//   }
// }
void ArduinoImplementation::_waitms(uint16_t ms)
{
    delay(ms);
}
uint8_t ArduinoImplementation::begin()
{
  interface->begin();
  return init();
}
}

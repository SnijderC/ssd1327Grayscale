#include "ssd1327Arduino.h"
//#include <Arduino.h>

Ssd1327Arduino::Ssd1327Arduino(TwoWire* i2c, uint8_t i2cAddress, uint8_t width, uint8_t height)
: Ssd1327(width, height), _i2c(i2c), _address(i2cAddress) {
    _i2c->begin();
}
void Ssd1327Arduino::_beginTransmission() {
  //Serial.printf("<STARTBIT> 0x%.2x", _address);
  _i2c->beginTransmission(_address);
}
void Ssd1327Arduino::_write(uint8_t data) {
  //Serial.printf(" 0x%.2x", data);
  _i2c->write(data);
}
uint8_t Ssd1327Arduino::_endTransmission() {
  //Serial.println(" <ENDBIT>");
  return _i2c->endTransmission(true);
}
void Ssd1327Arduino::_waitms(uint16_t ms) {
    delay(ms);
}
uint8_t Ssd1327Arduino::begin() {
  return init();
}

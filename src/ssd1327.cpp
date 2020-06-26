#include "ssd1327.h"
#include <stdlib.h>
#include <Arduino.h>

using namespace Ssd1327;

// Default implementation only returns false to indicate a software reset is
// required.
bool Interface::hwReset() {
  return false;
}

Implementation::Implementation(uint8_t width, uint8_t height):
  _width(width), _height(height) {}


uint8_t Implementation::setColumnRange(uint8_t start, uint8_t end) {
  start &= 0x3f; // 6-bit value
  end &= 0x3f; // 6-bit value
  uint8_t buffer[3] = {(uint8_t)Cmd::SetColumnRange, start, end};
  return interface->sendCommand(buffer, 3);
}
uint8_t Implementation::setRowRange(uint8_t start, uint8_t end) {
  start &= 0x7f; // 7-bit value
  end &= 0x7f; // 7-bit value
  uint8_t buffer[3] = {(uint8_t)Cmd::SetRowRange, start, end};
  return interface->sendCommand(buffer, 3);
}
uint8_t Implementation::resetRange() {
  uint8_t buffer[3] = {
    (uint8_t) Cmd::SetColumnRange, 0x00, (uint8_t) (_width / 2 -1)
  };
  return interface->sendCommand(buffer, 3);
  buffer[0] = (uint8_t)Cmd::SetRowRange;
  buffer[1] = _height - 1;
  return interface->sendCommand(buffer, 3);
}

uint8_t Implementation::setDisplayOff() {
  return interface->sendCommand((uint8_t)Cmd::DisplayOff);
}

uint8_t Implementation::setDisplayOn() {
  return interface->sendCommand((uint8_t)Cmd::DisplayOn);
}

uint8_t Implementation::setRemapping(
  bool comSplitOddEven,
  bool comRemapping,
  bool horizontalAddressIncrement,
  bool nibbleRemapping,
  bool gddrRemapping
) {
  uint8_t buffer[2] {(uint8_t)Cmd::SetRemapping, 0};
  if (comSplitOddEven)
    buffer[1] |= (uint8_t) Const::ComSplitOddEvenOnMask;
  if (comRemapping)
    buffer[1] |= (uint8_t) Const::ComRemappingOnMask;
  if (horizontalAddressIncrement)
    buffer[1] |= (uint8_t) Const::HorizontalAddressIncrementMask;
  if (nibbleRemapping)
    buffer[1] |= (uint8_t) Const::NibbleRemappingOnMask;
  if (gddrRemapping)
    buffer[1] |= (uint8_t) Const::GddrRemappingOnMask;
  return interface->sendCommand(buffer, 2);
}

uint8_t Implementation::resetRemapping() {
  return setRemapping(true, true, false, false, true);
}

uint8_t Implementation::setContrastLevel(uint8_t level) {
  uint8_t buffer[2] = {(uint8_t)Cmd::SetContrastLevel, level};
  return interface->sendCommand(buffer, 2);
}

uint8_t Implementation::setStartLine(uint8_t line) {
  line &= 0x7f; // 7-bit value
  uint8_t buffer[2] = {(uint8_t)Cmd::SetStartLine, line};
  return interface->sendCommand(buffer, 2);
}

uint8_t Implementation::setDisplayOffset(uint8_t offset) {
  offset &= 0x7f; // 7-bit value
  uint8_t buffer[2] = {(uint8_t)Cmd::SetDisplayOffset, offset};
  return interface->sendCommand(buffer, 2);
}

uint8_t Implementation::setDisplayNormal() {
  return interface->sendCommand((uint8_t)Cmd::SetDisplayNormal);
}

uint8_t Implementation::setDisplayAllOn() {
  return interface->sendCommand((uint8_t)Cmd::SetDisplayAllOn);
}

uint8_t Implementation::setDisplayAllOff() {
  return interface->sendCommand((uint8_t)Cmd::SetDisplayAllOff);
}

uint8_t Implementation::setDisplayInverse() {
  return interface->sendCommand((uint8_t)Cmd::SetDisplayInverse);
}

uint8_t Implementation::setMuxRatio(uint8_t ratio) {
  ratio &= 0x7f;
  uint8_t buffer[2] = {(uint8_t)Cmd::SetMuxRatio, ratio};
  return interface->sendCommand(buffer, 2);
}

uint8_t Implementation::resetMuxRatio() {
  return setMuxRatio(_height-1);
}

uint8_t Implementation::enableVddRegulator(bool state) {
  uint8_t _state = (state ? 1 : 0 );
   uint8_t buffer[2] = {(uint8_t)Cmd::EnableVddRegulator, _state};
  return interface->sendCommand(buffer, 2);
}

uint8_t Implementation::setPhaseLength(uint8_t phaseLen) {
  _phaseLen = phaseLen;
   uint8_t buffer[2] = {(uint8_t)Cmd::SetPhaseLength, phaseLen};
  return interface->sendCommand(buffer, 2);
}

uint8_t Implementation::setPixelResetPeriod(uint8_t period) {
  if (period < 1) period = 1;
  _phaseLen &= 0xf0;    // Reset lower order nibble.
  _phaseLen |= period;  // Set new lower order nibble.
  return setPhaseLength(_phaseLen);
}

uint8_t Implementation::setFirstPrechargePeriod(uint8_t period) {
  if (period < 1) period = 1;
  period <<= 4;        // Shift lower order nibble to higher order.
  _phaseLen &= 0x0f;   // Reset higher order nibble.
  _phaseLen |= period; // Set new higher order nibble.
  return setPhaseLength(_phaseLen);
}

uint8_t Implementation::setSecondPrechargePeriod(uint8_t period) {
  if (period < 1) period = 1;
  period &= 0x0f;
  uint8_t buffer[2] = {(uint8_t)Cmd::SetSecondPrechargePeriod, period};
  return interface->sendCommand(buffer, 2);
}

uint8_t Implementation::sendNoOp() {
  return interface->sendCommand((uint8_t)Cmd::SendNoOp);
}

uint8_t Implementation::setDisplayClock(uint8_t clock, uint8_t divider) {
  clock <<= 4;
  clock |= divider;
  uint8_t buffer[2] = {(uint8_t)Cmd::SetDisplayClock, clock};
  return interface->sendCommand(buffer, 2);
}

uint8_t Implementation::setGpioMode(GpioMode mode) {
  _gpioMode = mode;
  return setGpio(false);
}

uint8_t Implementation::setGpio(bool state) {
  if (_gpioMode != GpioMode::Output) return 255;
  uint8_t _state = (uint8_t) GpioMode::Output;
  if (state)
    _state |= 0b01;
  uint8_t buffer[2] = {(uint8_t)Cmd::SetGpio, _state};
  return interface->sendCommand(buffer, 2);
}

uint8_t Implementation::setGrayscaleLevels(uint8_t* grayScaleMap) {
  uint8_t buffer[16];
  buffer[0] = (uint8_t)Cmd::SetGrayscaleLevels;
  for (uint8_t i = 0; i < 14; i++) {
    buffer[i+1] = *(grayScaleMap+i);
  }
  return interface->sendCommand(buffer, 16);
}

uint8_t Implementation::resetGrayscale() {
  return interface->sendCommand((uint8_t)Cmd::resetGrayscale);
}

uint8_t Implementation::setPreChargeVoltage(uint8_t voltage) {
  uint8_t buffer[2] = {(uint8_t)Cmd::SetPreChargeVoltage, voltage};
  return interface->sendCommand(buffer, 2);
}

uint8_t Implementation::setComDeselectVoltage(uint8_t voltage) {
  uint8_t buffer[2] = {(uint8_t)Cmd::SetComDeselectVoltage, voltage};
  return interface->sendCommand(buffer, 2);
}

uint8_t Implementation::functionSelectionB(uint8_t selection) {
  selection |= (uint8_t) Const::FunctionSelectionBBase;
  uint8_t buffer[2] = {(uint8_t)Cmd::FunctionSelectionB, selection};
  return interface->sendCommand(buffer, 2);
}

uint8_t Implementation::enableSecondPrecharge(bool state) {
  _functionSelB &= 0b01; // Reset higher order bit
  if (!state) {
    _functionSelB |= 0b10; // Set higher order bit: *disables* second pre-charge
  }
  return functionSelectionB(_functionSelB);
}

uint8_t Implementation::enableVslRegulator(bool state) {
  _functionSelB &= 0b10; // Reset lower order bit
  if (!state) {
    _functionSelB |= 0b01; // Set lower order bit: *disables* Vsl regulator
  }
  return functionSelectionB(_functionSelB);
}

uint8_t Implementation::mcuProtectEnable() {
   uint8_t state = (
     (uint8_t)Const::McuProtectEnableBase | (uint8_t)Const::McuProtectLockMask
    );
  uint8_t buffer[2] = {(uint8_t)Cmd::McuProtectEnable, state};
  return interface->sendCommand(buffer, 2);
}

uint8_t Implementation::mcuProtectDisable() {
  uint8_t state = (uint8_t) Const::McuProtectEnableBase; // base is unlock
  uint8_t buffer[2] = {(uint8_t)Cmd::McuProtectEnable, state,};
  return interface->sendCommand(buffer, 2);
}

uint8_t Implementation::clear() {
  // Calculate number of bytes required to clear the display.
  // width * height / 2 => one nibble per pixel divided by buffer length
  uint16_t num_bytes = _width * _height / 2;
  // Calculate how many buffers we need to send to clear the display.
  // NOTE: Theoretically the buffer could bigger than num_bytes, that's ok, 0
  //       buffers will be sent.
  uint16_t num_buffers = num_bytes / (SSD1327_MAX_I2C_BUFFER - 2);
  // Calculate the remainder of bytes after the buffers are sent.
  num_bytes = num_bytes % (SSD1327_MAX_I2C_BUFFER - 2);

  // Clear by buffer length.
  for (uint16_t buf = 0; buf <= num_buffers; buf++)
  {
    interface->beginTransmission();
    interface->write(0x40);
    for (uint16_t i = 0; i < (SSD1327_MAX_I2C_BUFFER - 2); i++) {
      interface->write(0x00);
    }
    if(interface->endTransmission() != 0) return false;
  }
  interface->beginTransmission();
  // Clear remainder
  for (uint16_t i = 0; i < num_bytes; i++) {
    interface->write(0x00);
  }
  return interface->endTransmission() == 0;
}

uint8_t Implementation::getHeight()
{
  return _height;
}
uint8_t Implementation::getWidth()
{
  return _width;
}
/**
 * Can't just send data to the screen because a pixel is nibble and segments
 * are 2 pixels wide. If the image width is uneven, the last segment of each
 * line will contain both the last pixel of the lina and the first pixel of the
 * next. That will cause a skew in the image.
 */
uint8_t Implementation::renderImageData(
  uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t *image,
  uint16_t len
) {
  uint8_t error = 0;
  error = setRowRange(y, y + height - 1);
  if (error != 0) return error;
  error = setColumnRange(x / 2, (x + width) / 2 - 1 + (width % 2));
  if (error != 0) return error;
  error = setStartLine(0);
  if (error != 0) return error;
  if (width % 2 == 0) {
    return interface->sendData(image, len);
  }
  // This will be way less efficient because it requires several operations on
  // half of the image bytes.
  uint8_t bufLen = width / 2 + 1;
  uint8_t buffer[bufLen];
  for (uint16_t i = 0; i < len - 1; i += bufLen)
  {
    std::copy_n(image + i, bufLen, buffer);
    Serial.printf(
      "Buffer [i] 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
      buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]
    );
    if (i % 3 == 1)
    {
      for (uint8_t b = 0; b < bufLen; b++)
      {
        buffer[b] <<= 4;
        buffer[b] |= buffer[b+1] >> 4;
      }
    } else {
      i--;
    }
    buffer[bufLen - 1] &= 0xf0; // clear last 4 bits.
    Serial.printf(
      "Buffer [o] 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
      buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]
    );
    error = interface->sendData(buffer, bufLen);
    if (error != 0) return error;
  }
  return error;
}

// uint8_t Implementation::setHorizontalScrollRight() {
// }
// uint8_t Implementation::setHorizontalScrollLeft() {
// }
// uint8_t Implementation::activateScroll() {
// }
// uint8_t Implementation::dectivateScroll() {
//}

uint8_t Implementation::reset() {
  if (!interface->hwReset()) return 0;
  return clear();
}

// TODO: Reset everything, then set it again?
uint8_t Implementation::init() {
  uint8_t error = 0;
  error |= reset();
  // In case the display is in locked mode, unlock it.
  error |= mcuProtectDisable();
  // Turn it off for now.
  error |= setDisplayOff();
  // Enable Vdd regulator
  error |= enableVddRegulator(true);
  // Set MUX ration to amount of display lines.
  error |= resetMuxRatio();
  // Start rendering on line 1.
  error |= setStartLine((uint8_t) Default::StartLine);
  // Set rendering offset to 0.
  error |= setDisplayOffset((uint8_t) Default::DisplayOffset);
  // Only set necessary com to segment remapping (com split).
  error |= resetRemapping();
  // Set contrast to 50%.
  error |= setContrastLevel((uint8_t) Default::ContrastLevel);
  // Set current phase length value defaults to Default::PhaseLength.
  error |= setPhaseLength(_phaseLen);
  // Set the clock frequency and frequency divider.
  error |= setDisplayClock(
    (uint8_t) Default::DisplayClockFrequency,
    (uint8_t) Default::DisplayClockDivider
  );
  // Reset the gray scale colour map to the default.
  error |= resetGrayscale();
  // Set Pre-charge voltage of the OLED driver.
  error |= setPreChargeVoltage((uint8_t) Default::PreChargeVoltage);
  // Set com deselect voltage, for segments that should remain disabled.
  error |= setComDeselectVoltage((uint8_t) Default::ComDeselectVoltage);
  // Set second pre-charge period.
  error |= setSecondPrechargePeriod((uint8_t) Default::SecondPrechargePeriod);
  // Set current values for enabling/disabling second pre-charge and Vsl regulator.
  error |= functionSelectionB(_functionSelB);
  // Not inverse or all OLED on/off.
  error |= setDisplayNormal();
  // Rendering range to begin to end of screen.
  error |= resetRange();
  // Switch it on!
  error |= setDisplayOn();
  _waitms(100);
  return error;
}

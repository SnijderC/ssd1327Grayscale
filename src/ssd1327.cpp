#include "ssd1327.h"
#include <stdlib.h>
#include <algorithm>

Ssd1327::Ssd1327(uint8_t width, uint8_t height):
  _width(width), _height(height) {}


uint8_t Ssd1327::sendCommand(Cmd command, uint8_t *args, uint8_t argLen)
{
  _beginTransmission();
  _write(0x00); // Control byte 0x00 specifies commands.
  _write((uint8_t)command);
  while(argLen--)
  {
    _write(*args++);
  }
  return _endTransmission();
}

uint8_t Ssd1327::sendCommand(Cmd command)
{
  _beginTransmission();
  _write(0x00); // Control byte 0x00 specifies commands.
  _write((uint8_t)command);
  return _endTransmission();
}

uint8_t Ssd1327::sendData(uint8_t *data, uint16_t len)
{
  _beginTransmission();
  _write(0x40); // Control byte 0x40 specifies data.
  uint8_t error = 0;
  while (len--)
  {
    // Reset connection for every buffer length minus address and control byte
    if (len % (SSD1327_MAX_I2C_BUFFER - 2) == 0) {
      error = _endTransmission();
      if (error !=0) return error;
      _beginTransmission();
      _write(0x40);
    }
    _write(*data++);
  }
  return _endTransmission();
}

uint8_t Ssd1327::sendData(uint8_t data)
{
  _beginTransmission();
  _write(0x40); // Control byte 0x40 specifies data.
  _write(data);
  return _endTransmission();
}

uint8_t Ssd1327::setColumnRange(uint8_t start, uint8_t end) {
  start &= 0x3f; // 6-bit value
  end &= 0x3f; // 6-bit value
  uint8_t buffer[2] = {start, end};
  return sendCommand(Cmd::SetColumnRange, buffer, 2);
}
uint8_t Ssd1327::setRowRange(uint8_t start, uint8_t end) {
  start &= 0x7f; // 7-bit value
  end &= 0x7f; // 7-bit value
  uint8_t buffer[2] = {start, end};
  return sendCommand(Cmd::SetRowRange, buffer, 2);
}
uint8_t Ssd1327::resetRange() {
  uint8_t buffer[2] = { 0x00, (uint8_t) (_width / 2 -1) };
  return sendCommand(Cmd::SetColumnRange, buffer, 2);
  buffer[1] = _height - 1;
  return sendCommand(Cmd::SetRowRange, buffer, 2);
}

uint8_t Ssd1327::setDisplayOff() {
  return sendCommand(Cmd::DisplayOff);
}

uint8_t Ssd1327::setDisplayOn() {
  return sendCommand(Cmd::DisplayOn);
}

uint8_t Ssd1327::setRemapping(
  bool comSplitOddEven,
  bool comRemapping,
  bool horizontalAddressIncrement,
  bool nibbleRemapping,
  bool gddrRemapping
) {
  uint8_t remap = 0;
  if (comSplitOddEven)
    remap |= (uint8_t) Const::ComSplitOddEvenOnMask;
  if (comRemapping)
    remap |= (uint8_t) Const::ComRemappingOnMask;
  if (horizontalAddressIncrement)
    remap |= (uint8_t) Const::HorizontalAddressIncrementMask;
  if (nibbleRemapping)
    remap |= (uint8_t) Const::NibbleRemappingOnMask;
  if (gddrRemapping)
    remap |= (uint8_t) Const::GddrRemappingOnMask;
  return sendCommand(Cmd::SetRemapping, &remap, 1);
}

uint8_t Ssd1327::resetRemapping() {
  return setRemapping(true, true, false, false, true);
}

uint8_t Ssd1327::setContrastLevel(uint8_t level) {
  return sendCommand(Cmd::SetContrastLevel, &level, 1);
}

uint8_t Ssd1327::setStartLine(uint8_t line) {
  line &= 0x7f; // 7-bit value
  return sendCommand(Cmd::SetStartLine, &line, 1);
}

uint8_t Ssd1327::setDisplayOffset(uint8_t offset) {
  offset &= 0x7f; // 7-bit value
  return sendCommand(Cmd::SetDisplayOffset, &offset, 1);
}

uint8_t Ssd1327::setDisplayNormal() {
  return sendCommand(Cmd::SetDisplayNormal);
}

uint8_t Ssd1327::setDisplayAllOn() {
  return sendCommand(Cmd::SetDisplayAllOn);
}

uint8_t Ssd1327::setDisplayAllOff() {
  return sendCommand(Cmd::SetDisplayAllOff);
}

uint8_t Ssd1327::setDisplayInverse() {
  return sendCommand(Cmd::SetDisplayInverse);
}

uint8_t Ssd1327::setMuxRatio(uint8_t ratio) {
  ratio &= 0x7f;
  return sendCommand(Cmd::SetMuxRatio, &ratio, 1);
}

uint8_t Ssd1327::resetMuxRatio() {
  return setMuxRatio(_height-1);
}

uint8_t Ssd1327::enableVddRegulator(bool state) {
  uint8_t _state = (state ? 1 : 0 );
  return sendCommand(Cmd::EnableVddRegulator, &_state, 1);
}

uint8_t Ssd1327::setPhaseLength(uint8_t phaseLen) {
  _phaseLen = phaseLen;
  return sendCommand(Cmd::SetPhaseLength, &phaseLen, 1);
}

uint8_t Ssd1327::setPixelResetPeriod(uint8_t period) {
  if (period < 1) period = 1;
  _phaseLen &= 0xf0;    // Reset lower order nibble.
  _phaseLen |= period;  // Set new lower order nibble.
  return setPhaseLength(_phaseLen);
}

uint8_t Ssd1327::setFirstPrechargePeriod(uint8_t period) {
  if (period < 1) period = 1;
  period <<= 4;        // Shift lower order nibble to higher order.
  _phaseLen &= 0x0f;   // Reset higher order nibble.
  _phaseLen |= period; // Set new higher order nibble.
  return setPhaseLength(_phaseLen);
}

uint8_t Ssd1327::setSecondPrechargePeriod(uint8_t period) {
  if (period < 1) period = 1;
  period &= 0x0f;
  return sendCommand(Cmd::SetSecondPrechargePeriod, &period, 1);
}

uint8_t Ssd1327::sendNoOp() {
  return sendCommand(Cmd::SendNoOp);
}

uint8_t Ssd1327::setDisplayClock(uint8_t clock, uint8_t divider) {
  clock <<= 4;
  clock |= divider;
  return sendCommand(Cmd::SetDisplayClock, &clock, 1);
}

uint8_t Ssd1327::setGpioMode(GpioMode mode) {
  _gpioMode = mode;
  return setGpio(false);
}

uint8_t Ssd1327::setGpio(bool state) {
  if (_gpioMode != GpioMode::Output) return 255;
  uint8_t _state = (uint8_t) GpioMode::Output;
  if (state)
    _state |= 0b01;
  return sendCommand(Cmd::SetGpio, &_state, 1);
}

uint8_t Ssd1327::setGrayscaleLevels(uint8_t* grayScaleMap) {
  return sendCommand(Cmd::SetGrayscaleLevels, grayScaleMap, 15);
}

uint8_t Ssd1327::resetGrayscale() {
  return sendCommand(Cmd::resetGrayscale);
}

uint8_t Ssd1327::setPreChargeVoltage(uint8_t voltage) {
  return sendCommand(Cmd::SetPreChargeVoltage, &voltage, 1);
}

uint8_t Ssd1327::setComDeselectVoltage(uint8_t voltage) {
  return sendCommand(Cmd::SetComDeselectVoltage, &voltage, 1);
}

uint8_t Ssd1327::functionSelectionB(uint8_t selection) {
  selection |= (uint8_t) Const::FunctionSelectionBBase;
  return sendCommand(Cmd::FunctionSelectionB, &selection, 1);
}

uint8_t Ssd1327::enableSecondPrecharge(bool state) {
  _functionSelB &= 0b01; // Reset higher order bit
  if (!state) {
    _functionSelB |= 0b10; // Set higher order bit: *disables* second pre-charge
  }
  return functionSelectionB(_functionSelB);
}

uint8_t Ssd1327::enableVslRegulator(bool state) {
  _functionSelB &= 0b10; // Reset lower order bit
  if (!state) {
    _functionSelB |= 0b01; // Set lower order bit: *disables* Vsl regulator
  }
  return functionSelectionB(_functionSelB);
}

uint8_t Ssd1327::mcuProtectEnable() {
   uint8_t state = (
     (uint8_t)Const::McuProtectEnableBase | (uint8_t)Const::McuProtectLockMask
    );
  return sendCommand(Cmd::McuProtectEnable, &state, 1);
}

uint8_t Ssd1327::mcuProtectDisable() {
  uint8_t state = (uint8_t) Const::McuProtectEnableBase; // base is unlock
  return sendCommand(Cmd::McuProtectEnable, &state, 1);
}

uint8_t Ssd1327::clear() {
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
    _beginTransmission();
    _write(0x40);
    for (uint16_t i = 0; i < (SSD1327_MAX_I2C_BUFFER - 2); i++) {
      _write(0x00);
    }
    if(_endTransmission() != 0) return false;
  }
  _beginTransmission();
  // Clear remainder
  for (uint16_t i = 0; i < num_bytes; i++) {
    _write(0x00);
  }
  return _endTransmission() == 0;
}

uint8_t Ssd1327::getHeight()
{
  return _height;
}
uint8_t Ssd1327::getWidth()
{
  return _width;
}
/**
 * Can't just send data to the screen because a pixel is nibble and segments
 * are 2 pixels wide. If the image width is uneven, the last segment of each
 * line will contain both the last pixel of the lina and the first pixel of the
 * next. That will cause a skew in the image.
 */
uint8_t Ssd1327::renderImageData(
  uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t *image,
  uint16_t len
) {
  uint8_t error = 0;
  error = setColumnRange(x / 2, (x + width) / 2 - 1);
  if (error != 0) return error;
  error = setRowRange(y, y + height - 1);
  if (error != 0) return error;
  error = setStartLine(0);
  if (error != 0) return error;
  if (width % 2 == 0) {
    return sendData(image, len);
  }
  // This will be way less efficient because it requires several operations on
  // half of the image bytes.
  uint8_t bufLen = width / 2 + 1;
  uint8_t buffer[bufLen];
  for (uint16_t i = 0; i < len; i += width / 2)
  {
    std::copy_n(image + i, bufLen, buffer);
    if (i % 2 == 1)
    {
      for (uint8_t b = 0; b < bufLen; b++)
      {
        buffer[b] <<= 4;
        buffer[b] |= buffer[b+1] >> 4;
      }
    }
    buffer[bufLen - 1] &= 0xf0; // clear last 4 bits.
    error = sendData(buffer, bufLen);
    if (error != 0) return error;
  }
  return error;
}

// uint8_t Ssd1327::setHorizontalScrollRight() {
// }
// uint8_t Ssd1327::setHorizontalScrollLeft() {
// }
// uint8_t Ssd1327::activateScroll() {
// }
// uint8_t Ssd1327::dectivateScroll() {
//}

// TODO: Reset everything, then set it again?
uint8_t Ssd1327::init() {
  uint8_t error = 0;
  // In case the display is in locked mode, unlock it.
  error |= mcuProtectDisable();
  // Turn it off to clear the display.
  error |= setDisplayOff();
  // Set MUX ration to amount of display lines.
  error |= resetMuxRatio();
  // Start rendering on line 1.
  error |= setStartLine((uint8_t) Default::StartLine);
  // Set rendering offset to 0.
  error |= setDisplayOffset((uint8_t) Default::DisplayOffset);
  // Only set necessary com to segment remapping (com split).
  error |= resetRemapping();
  // Enable Vdd regulator
  error |= enableVddRegulator(true);
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

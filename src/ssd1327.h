/*
 * SSD1327 Grayscale driver library for SSD1327 over I2C.
 *
 */
#ifndef SSD1327_MAX_I2C_BUFFER
#define SSD1327_MAX_I2C_BUFFER 32
#endif

#include <stdint.h>

class Ssd1327  {

public:

  /**
   * SSD1327 Commands.
   *
   * Enumeration of all SSD1327 commands, description in comments. If the
   * command needs arguments, it shows what arguments are expected too. E.g.:
   *
   *      arg1 A[4:0], arg2 B[3:0] arg3 B[4:7]
   *
   * In the brackets is the valid range for the argument, interpret the numbers
   * as the starting bit from lowest order to highest, followed by the end bit.
   * Thus the example above shows arg1 a having a 5 bit value (0x00 - 0x1f),
   * arg2 is placed in the 4 lowest order nibble of the
   */
  enum class Cmd: uint8_t {
    // Set column start and end addresses: start A[6:0], end B[5:0].
    SetColumnRange          = 0x15,
    // Set row start and end addresses: start A[5:0], end B[5:0].
    SetRowRange             = 0x75,
    // Set contrast: contrast A[7:0]
    SetContrastLevel        = 0x81,
    /** Set remapping of GDDRAM: [ 0b A6 0b A4 0b A2 A1 A0 ]
     *  A[0]:
     *    0b, Column address remap off
     *    1b, Column address remap on (swaps the rendering direction from right
     *        to left, to left to right. Use with Nibble remapping to mirror
     *        the image.
     *  A[1]: Nibble remapping
     *    0b, Off
     *    1b, On (swaps first and last 4 bits of a byte, effectively swapping 2
     *        pixels). Use with column remapping to mirror the image.
     *  A[2]: Address increment mode
     *    0b, Horizontal address increment, a.k.a. renders lines left to right,
     *        top to bottom.
     *    1b, Vertical address increment, a.k.a. render lines top to bottom,
     *        left to right.
     *  A[4]: Com remapping
     *    0b, Off
     *    1b, On
     *  A[6]: COM Split Odd Even, the controller is connected to the display
     *        lines from the left and right and right side. The left and right
     *        side coms alternate lines, the left side coms are connected to
     *        uneven display rows, the right side coms are connected to the
     *        even rows, but the numbering of the controller bus is sequential.
     *        The controller com lines are though split through the middle,
     *        The left side coms from left to right are 127 - 64, the right
     *        side coms from left to right are 0 - 63. In the default mode (0b)
     *        the lines are interleaved to match the com lines with the display
     *        lines, when set to off (1b) your lines will be displayed in the
     *        wrong order, in other words you will have to interleave the image
     *        yourself.
     *    0b, On (default, good)
     *    1b, Off (bad)
     * Use these with the remapping enum and apply an OR operator to combine
     * them.
     */
    SetRemapping            = 0xa0,
    // Set display start line: start [6:0] (128 lines, 0 - 127).
    SetStartLine            = 0xa1,
    /**
     * Set display offset, changes vertical memory (row) to display line (com)
     * mapping. E.g. set it to 40, row 0 in memory will be mapped to com39 on
     * the display shifting the image down and wrapping around to show the
     * bottom of the image at the top of the diplay, above display line 39.
     * All this assumes you have MUX set to the default of 128 and you didn't
     * use remap commands: offset [6:0].
     */
    SetDisplayOffset        = 0xa2,
    // Normal diplay mode.
    SetDisplayNormal        = 0xa4,
    // Power on all OLEDs, max power.
    SetDisplayAllOn         = 0xa5,
    // Power off all OLEDs.
    SetDisplayAllOff        = 0xa6,
    // Inverted display mode, considers grayscale.
    SetDisplayInverse       = 0xa7,
    /**
     * Set multiplex ratio how many lines the display should multiplex. In
     * effect it controls how many rows are rendered. Combine with
     * SetDisplayOffset to render only part of the display: [ 0x10 - 0x80 ],
     * corrresponds to 16MUX-128MUX apply required mux ratio -1, e.g. send 127
     * for 128MUX (default), values under 15 are invalid: ratio [6:0].
     */
    SetMuxRatio             = 0xa8,
    /**
     * Enable/Disable the internal VDD regulator (for internal core logic
     * operation). This is required to match the wiring of the display.
     * enable A[0:0]
     */
    EnableVddRegulator      = 0xab,
    // Switch the display "off" (sleep mode).
    DisplayOff              = 0xae,
    // Switch the display on.
    DisplayOn               = 0xaf,
    /**
     * Set phase length of pre charging circuit for driving OLEDs: two nibbles
     * for setting the duration of resetting (phase 1) and pre charging
     * (phase 2) the OLED capacitor, in steps from 1 - 15 DCLK cycles.
     * 0 is not a valid reset / pre charge phase time. PWM set in GS15 should
     * be larger than the precharge cycles added up. The required phase length
     * depends on the capacity of the capacitor on the OLED, if it has higher
     * capacity, (dis)charging takes longer:
     * reset A[3:0], precharge a[7:4].
     */
    SetPhaseLength          = 0xb1,
    // Send the GPU a NOP (no operation, do nothing instruction).
    SendNoOp                = 0xb2,
    /**
     * Set display clock frequency (DCLK) and frequency divider.
     * Sets the frequency of rendering pixels, and the frequency divider.
     * The DCLK can be set between 535 and 655KHz, in 16 levels (4 bits). The
     * divide ratio can be set in 16 levels as well: divide A[3:0],
     * frequency A[7:4].
     */
    SetDisplayClock         = 0xb3,
    // Set the GPIO to, low (00b), Hi-Z (01b) or high (11b).: gpio A[1:0].
    SetGpio                 = 0xb5,
    /**
     * Set second pre-charge period, requires second pre-charge to be enabled
     * by the enable functionSelection command. Second pre charge time can be
     * set to 0 to 15 DCLKs: precharge A[3:0]
     * This is the third phase of driving pixels.
     */
    SetSecondPrechargePeriod = 0xb6,
    /**
     * Set grayscale table, sets the brightness of each of the 16 selectable
     * levels of the grayscale. The brightness is set in pulse width of 6 bit
     * pulses of the DCLK, level zero is always zero, you can set the other
     * 15 levels per byte. This controls phase 4 of driving pixels: 15x A[5:0].
     */
    SetGrayscaleLevels      = 0xb8, // Controls brightness
    /**
     * Reset grayscale levels to linear default (GS0: 0, GS1: 0, GS2: 2 - for
     * GS3 - GS14. Also resets phase 4 of pixel driving to default:
     * `(GS-1)*2`, GS15 28).
     */
    resetGrayscale          = 0xb9, // Default brightness
    // Send the GPU a NOP (no operation, do nothing instruction).
    // SendNoOp         = 0xbb,
    /**
     * Set precharge voltage level relative to VCC, on a non-lineary scale
     * This is the voltage applied to OLED pixels.:level A[3:0], example
     * values:
     * 0x00: 0.2 * VCC
     *  ...
     * 0x05: 0.5 * VCC (default)
     * ...
     * 0x07: 0.613 * VCC
     * 0x08: VCOMH
     *
     */
    SetPreChargeVoltage     = 0xbc,
    /**
     * Set COM deselect voltage level relative to VCC, on a non-lineary scale:
     * level A[2:0], example values:
     * 0x00: 0.72 * VCC
     *  ...
     * 0x05: 0.82 * VCC (default)
     * ...
     * 0x07: 0.86 * VCC
     * This voltage should be sufficiently high such that the OLED bias is
     * reversed (in which state it can't emit light). This voltage supply is
     * used to reverse all OLEDs in a segment that is deselected.
     */
    SetComDeselectVoltage   = 0xbe,
    /**
     * Enable/Disable second pre-charge (phase 4) of the OLED driver.
     * Enable/Disable controls whether the low level voltage reference (Vsl) of
     * the driver should be regulated internally or externally:
     * disable A[0:0], vsl A[1:1]
     */
    FunctionSelectionB      = 0xd5,
    /**
     * Makes the display controller ignore any command expect for the unlock
     * command (same command with different argument). Use
     * `Ssd1327::Const::McuProtectEnable` to construct the argument, OR is with
     * 0x04 to enable the lock, or send it as is, to unlock it:
     * lock A[3:0] OR Ssd1327::Const::McuProtectEnable.
     */
    McuProtectEnable        = 0xfd,
    /**
     * Set the parameters for scrolling horizontally: pixel range and timing.
     * You can scroll any range on the display by specifying the start and end
     * rows and columns. You can set the scroll speed relative to the frame
     * rate:
     * dummy       A[7:0]: mandatory empty byte,
     * rowStart    B[6:0]: 0x00 - 0x7f (row 1 - 128)
     * interval    C[2:0]: 3 bit interval according to a predefined table.
     *                     The interval is specified by an amount of frames, so
     *                     it depends on the framerate. You can use enum
     *                     ScrollSpeed to set the interval.
     * rowEnd      D[6:0]  0x00 - 0x7f (row 1 - 128), > rowStart
     * columnStart E[5:0]: 0x00 - 0x3f (column 1 - 64)
     *
     * columnEnd   F[5:0]: 0x00 - 0x3f (column 1 - 64) > columnStart
     */
    SetHorizontalScrollRight= 0x26,
    // Same as SetHorizontalScrollRight, except it scrolls left.
    SetHorizontalScrollLeft = 0x27,
    ActivateScroll          = 0x2f,
    DectivateScroll         = 0x2e
  };

  /**
   * SSD1327 constants.
   *
   * Enumeration of all SSD1327 constants, description in comments.
   */
  enum class Const: uint8_t {
    // Protect command needs to be this value ORed with 0x04 to enable or as is
    // to disable protection.
     McuProtectEnableBase           = 0b00010010,
     McuProtectLockMask             = 0b00000100,
     ComSplitOddEvenOnMask          = 0b01000000,
     ComRemappingOnMask             = 0b00010000,
     HorizontalAddressIncrementMask = 0b01000100,
     NibbleRemappingOnMask          = 0b00000010,
     GddrRemappingOnMask            = 0b00000001,
     FunctionSelectionBBase         = 0b01100000,
  };

  /**
   * SSD1327 default values from the manual.
   */
  enum class Default: uint8_t {
    ContrastLevel         = 0xff,
    Remapping             = (uint8_t)Const::ComSplitOddEvenOnMask,
    StartLine             = 0x00,
    DisplayOffset         = 0x00,
    PhaseLength           = 0xf1,
    PixelResetPeriod      = 0xf0,
    FirstPrechargePeriod  = 0x01,
    SecondPrechargePeriod = 0x04,
    DisplayClockFrequency = 0x0f,
    DisplayClockDivider   = 0x00,
    PreChargeVoltage      = 0x05,
    ComDeselectVoltage    = 0x05,
    FunctionSelectionB    = 0x62,
  };

  /**
   * Horizontal Scrollspeed constants.
   */
  enum class ScrollSpeed: uint8_t {
    Frames_2      = 0b111,
    Frames_3      = 0b100,
    Frames_4      = 0b101,
    Frames_5      = 0b110,
    Frames_6      = 0b000,
    Frames_32     = 0b001,
    Frames_64     = 0b010,
    Frames_256    = 0b011
  };

  enum class GpioMode: uint8_t {
    InputDisable = 0b00,
    InputEnable  = 0b01,
    Output       = 0b10
  };

  /**
   * Create an instance of the OLED driver.
   *
   * @param Ssd1327::Interface OLED interface struct.
   */
  Ssd1327(uint8_t width, uint8_t height);
  uint8_t setColumnRange(uint8_t start, uint8_t end);
  uint8_t setRowRange(uint8_t start, uint8_t end);
  uint8_t resetRange();
  uint8_t setDisplayOff();
  uint8_t setDisplayOn();
  uint8_t setDisplayOffset(uint8_t offset);
  uint8_t setDisplayNormal();
  uint8_t setDisplayAllOn();
  uint8_t setDisplayAllOff();
  uint8_t setDisplayInverse();

  uint8_t setContrastLevel(uint8_t level);
  uint8_t setRemapping(
    bool comSplitOddEven,
    bool comRemapping,
    bool horizontalAddressIncrement,
    bool nibbleRemapping,
    bool gddrRemapping
  );
  uint8_t resetRemapping();
  uint8_t setStartLine(uint8_t line);

  uint8_t setMuxRatio(uint8_t ratio);
  uint8_t resetMuxRatio();

  uint8_t enableVddRegulator(bool state);
  uint8_t setPhaseLength(uint8_t phaseLen);
  /**
   * Set the time to discharge the OLED pixel capacitor.
   *
   * Note: this does not map to an exact command in the manual because this
   * setting is combined with the first pre-charge period in one byte and send
   * by the Cmd::SetPhaseLength command. If you need to set both these values
   * it's better to use the setPhaseLength function because it will send both
   * values to the module in 1 write action.
   * @param period for the OlED drivers first pre charge, only low 4 lowest
   *        order bits are considered, minium value: 1.
   */
  uint8_t setPixelResetPeriod(uint8_t period);
  /**
   * Set the time to pre charge the OLED pixel driver.
   *
   * Note: this does not map to an exact command in the manual because this
   * setting is combined with the pixel reset period in one byte and send by
   * the Cmd::SetPhaseLength command. If you need to set both these values it's
   * better to use the setPhaseLength function because it will send both values
   * to the module in 1 write action.
   * @param period for the OlED drivers first pre charge, only low 4 lowest
   *        order bits are considered, minium value: 1.
   */
  uint8_t setFirstPrechargePeriod(uint8_t period);
  uint8_t setSecondPrechargePeriod(uint8_t period);
  uint8_t sendNoOp();
  uint8_t setDisplayClock(uint8_t clock, uint8_t divider);
  uint8_t setGpio(bool state);
  uint8_t setGpioMode(GpioMode mode);
  uint8_t setGrayscaleLevels(uint8_t* grayScaleMap);
  uint8_t resetGrayscale();
  uint8_t setPreChargeVoltage(uint8_t voltage);
  uint8_t setComDeselectVoltage(uint8_t voltage);
  uint8_t functionSelectionB(uint8_t selection);
  uint8_t enableSecondPrecharge(bool state);
  uint8_t enableVslRegulator(bool state);
  uint8_t mcuProtectEnable();
  uint8_t mcuProtectDisable();
  // uint8_t setHorizontalScrollRight();
  // uint8_t setHorizontalScrollLeft();
  // uint8_t activateScroll();
  // uint8_t dectivateScroll();

  uint8_t sendCommand(Cmd command, uint8_t *args, uint8_t argLen);
  uint8_t sendCommand(Cmd command);
  uint8_t sendData(uint8_t *data, uint16_t len);
  uint8_t sendData(uint8_t command);
  uint8_t clear();
  uint8_t init();

private:
  uint8_t _width;
  uint8_t _height;
  uint8_t _phaseLen     = (uint8_t) Default::PhaseLength;
  uint8_t _functionSelB = (uint8_t) Default::FunctionSelectionB;
  GpioMode _gpioMode;
  /**
   * Virtual method that should implement writing data to the display via I2C
   * SPI, 6800, etc.
   *
   * @param data pointer to bytes to send to the module.
   * @param len of data.
   */
  virtual void _write(uint8_t data)=0;
  virtual void _beginTransmission()=0;
  virtual uint8_t _endTransmission()=0;
  /**
   * Platform independent wait function.
   * Apply a wait/delay/sleep/timer interrupt that suits your platform.
   *
   * @param ms to wait.
   */
  virtual void _waitms(uint16_t ms)=0;
};

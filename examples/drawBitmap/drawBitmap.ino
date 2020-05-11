#include <Arduino.h>
#include <Wire.h>
#include <ssd1327Arduino.h>
#include "gscale.h"
#define SCL 4
#define SDA 2
#define I2C_FREQUENCY 400000L

Ssd1327Arduino oled(&Wire, 0x3c, 128, 128);

void setup() {
    Wire.begin(SDA, SCL, I2C_FREQUENCY);
    oled.begin();
    oled.resetRange();
    oled.setStartLine(0);
    oled.clear();
    oled.renderImageData(
        0, 0, 128, 128, (uint8_t*)gscale_bits, sizeof(gscale_bits)
    );
}

void loop() {}

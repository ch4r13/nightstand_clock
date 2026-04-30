#include "touch_cst816.h"

#define REG_CHIP_ID 0xA7

CST816Touch::CST816Touch(uint8_t sda, uint8_t scl, uint8_t irq, uint8_t rst)
    : _sda(sda), _scl(scl), _irq(irq), _rst(rst) {}

bool CST816Touch::begin() {
    pinMode(_irq, INPUT);
    reset();

    Wire.begin(_sda, _scl);
    Wire.setClock(400000);

    // Check device is present
    Wire.beginTransmission(CST816_I2C_ADDR);
    if (Wire.endTransmission() != 0) return false;

    // Read chip ID for diagnostics only – proceed regardless of value
    Wire.beginTransmission(CST816_I2C_ADDR);
    Wire.write(REG_CHIP_ID);
    Wire.endTransmission(false);
    Wire.requestFrom(CST816_I2C_ADDR, (uint8_t)1);
    if (Wire.available())
        Serial.printf("[Touch] chip ID 0x%02X\n", Wire.read());

    return true;
}

bool CST816Touch::available() {
    return digitalRead(_irq) == LOW;
}

// Read 7 bytes straight from address 0x00 (no register-select write).
// The chip auto-resets its register pointer to 0x00 after each transaction,
// matching the behaviour the CST816D reference library relies on.
bool CST816Touch::read(TouchPoint &pt) {
    Wire.requestFrom(CST816_I2C_ADDR, (uint8_t)7);
    if (Wire.available() < 7) { pt.pressed = false; return false; }

    Wire.read();                               // 0x00 – reserved
    pt.gesture  = (CST816Gesture)Wire.read();  // 0x01 – gesture ID
    uint8_t fingers = Wire.read();             // 0x02 – finger count
    uint8_t xh  = Wire.read();                 // 0x03 – XposH
    uint8_t xl  = Wire.read();                 // 0x04 – XposL
    uint8_t yh  = Wire.read();                 // 0x05 – YposH
    uint8_t yl  = Wire.read();                 // 0x06 – YposL

    pt.pressed = (fingers > 0);
    pt.x = ((int16_t)(xh & 0x0F) << 8) | xl;
    pt.y = ((int16_t)(yh & 0x0F) << 8) | yl;
    return pt.pressed;
}

void CST816Touch::reset() {
    if (_rst == 0xFF) return;
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW);  delay(20);
    digitalWrite(_rst, HIGH); delay(200);
}

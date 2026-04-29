#include "touch_cst816.h"

#define REG_GESTURE   0x01
#define REG_FINGERS   0x02
#define REG_XH        0x03
#define REG_XL        0x04
#define REG_YH        0x05
#define REG_YL        0x06
#define REG_CHIP_ID   0xA7
#define REG_MOTION    0xEC
#define REG_IRQ_CTL   0xFA

CST816Touch::CST816Touch(uint8_t sda, uint8_t scl, uint8_t irq, uint8_t rst)
    : _sda(sda), _scl(scl), _irq(irq), _rst(rst) {}

bool CST816Touch::begin() {
    pinMode(_irq, INPUT_PULLUP);
    reset();

    Wire.begin(_sda, _scl);
    Wire.setClock(400000);

    Wire.beginTransmission(CST816_I2C_ADDR);
    Wire.write(REG_CHIP_ID);
    if (Wire.endTransmission(false) != 0) return false;
    Wire.requestFrom(CST816_I2C_ADDR, (uint8_t)1);
    if (!Wire.available()) return false;
    uint8_t id = Wire.read();
    if (id != 0xB4 && id != 0xB5 && id != 0xB6) return false;

    Wire.beginTransmission(CST816_I2C_ADDR);
    Wire.write(REG_MOTION);
    Wire.write(0x71);
    Wire.endTransmission();

    Wire.beginTransmission(CST816_I2C_ADDR);
    Wire.write(REG_IRQ_CTL);
    Wire.write(0x41);
    Wire.endTransmission();

    return true;
}

bool CST816Touch::available() {
    return digitalRead(_irq) == LOW;
}

bool CST816Touch::read(TouchPoint &pt) {
    Wire.beginTransmission(CST816_I2C_ADDR);
    Wire.write(REG_GESTURE);
    if (Wire.endTransmission(false) != 0) { pt.pressed = false; return false; }
    Wire.requestFrom(CST816_I2C_ADDR, (uint8_t)6);
    if (Wire.available() < 6) { pt.pressed = false; return false; }

    pt.gesture  = (CST816Gesture)Wire.read();
    uint8_t fingers = Wire.read();
    uint8_t xh  = Wire.read();
    uint8_t xl  = Wire.read();
    uint8_t yh  = Wire.read();
    uint8_t yl  = Wire.read();

    pt.pressed = (fingers > 0);
    pt.x = ((int16_t)(xh & 0x0F) << 8) | xl;
    pt.y = ((int16_t)(yh & 0x0F) << 8) | yl;
    return pt.pressed;
}

void CST816Touch::reset() {
    if (_rst == 0xFF) return;
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW);  delay(10);
    digitalWrite(_rst, HIGH); delay(50);
}

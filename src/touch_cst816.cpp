#include "touch_cst816.h"

CST816Touch::CST816Touch(uint8_t sda, uint8_t scl, uint8_t irq, uint8_t rst)
    : _sda(sda), _scl(scl), _irq(irq), _rst(rst) {}

bool CST816Touch::begin() {
    // Match CST816D library order: Wire first, then hardware reset
    Wire.begin(_sda, _scl);
    Wire.setClock(400000);

    reset();   // RST LOW 20 ms → HIGH, then 400 ms settle

    // ---- I2C bus scan (diagnostics) ----
    Serial.print("[Touch] I2C scan:");
    bool found = false;
    for (uint8_t addr = 1; addr < 0x7F; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf(" 0x%02X", addr);
            if (addr == CST816_I2C_ADDR) found = true;
        }
    }
    Serial.println();

    if (!found) {
        Serial.printf("[Touch] not found at 0x%02X\n", CST816_I2C_ADDR);
        return false;
    }

    // Read chip ID for information
    Wire.beginTransmission(CST816_I2C_ADDR);
    Wire.write(0xA7);
    Wire.endTransmission(false);
    Wire.requestFrom(CST816_I2C_ADDR, (uint8_t)1);
    if (Wire.available())
        Serial.printf("[Touch] chip ID 0x%02X\n", Wire.read());

    pinMode(_irq, INPUT);
    return true;
}

bool CST816Touch::available() {
    return digitalRead(_irq) == LOW;
}

// Direct requestFrom, no preceding register-address write.
// The chip resets its register pointer to 0x00 after each transaction
// (same assumption as the CST816D reference library).
bool CST816Touch::read(TouchPoint &pt) {
    uint8_t n = Wire.requestFrom(CST816_I2C_ADDR, (uint8_t)7);
    if (n < 3 || Wire.available() < 3) { pt.pressed = false; return false; }

    uint8_t raw[7] = {};
    for (uint8_t i = 0; i < n && i < 7; i++) raw[i] = Wire.read();

    pt.gesture  = (CST816Gesture)raw[1];   // 0x01 gesture ID
    uint8_t fingers = raw[2];              // 0x02 finger count
    uint8_t xh  = raw[3];                  // 0x03 XposH
    uint8_t xl  = raw[4];                  // 0x04 XposL
    uint8_t yh  = raw[5];                  // 0x05 YposH
    uint8_t yl  = raw[6];                  // 0x06 YposL

    pt.pressed = (fingers > 0);
    pt.x = ((int16_t)(xh & 0x0F) << 8) | xl;
    pt.y = ((int16_t)(yh & 0x0F) << 8) | yl;
    return true;   // return true = data received (caller checks pt.pressed)
}

void CST816Touch::reset() {
    if (_rst == 0xFF) return;
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW);  delay(20);
    digitalWrite(_rst, HIGH); delay(400);  // 400 ms – matches CST816D
}

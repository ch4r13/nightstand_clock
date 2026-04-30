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

bool CST816Touch::read(TouchPoint &pt) {
    // Explicitly set register pointer to 0x00 before each read.
    // Without this the pointer stays where the last transaction left it
    // (e.g. 0xA8 after the chip-ID read in begin()), so all subsequent
    // requestFrom calls read the wrong registers and return garbage.
    Wire.beginTransmission(CST816_I2C_ADDR);
    Wire.write(0x00);
    if (Wire.endTransmission(false) != 0) {   // repeated-start, keep bus
        pt.pressed = false;
        return false;   // chip NAK'd (sleeping / not present)
    }

    uint8_t n = Wire.requestFrom((uint8_t)CST816_I2C_ADDR, (uint8_t)7);
    if (n < 7) { pt.pressed = false; return false; }

    uint8_t raw[7];
    for (uint8_t i = 0; i < 7; i++) raw[i] = Wire.read();

    pt.gesture  = (CST816Gesture)raw[1];          // reg 0x01
    uint8_t fingers = raw[2];                      // reg 0x02
    pt.pressed = (fingers > 0);
    pt.x = ((int16_t)(raw[3] & 0x0F) << 8) | raw[4];   // regs 0x03-0x04
    pt.y = ((int16_t)(raw[5] & 0x0F) << 8) | raw[6];   // regs 0x05-0x06
    return true;
}

void CST816Touch::reset() {
    if (_rst == 0xFF) return;
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW);  delay(20);
    digitalWrite(_rst, HIGH); delay(400);  // 400 ms – matches CST816D
}

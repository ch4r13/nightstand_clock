#pragma once
#include <Arduino.h>
#include <Wire.h>

#define CST816_I2C_ADDR  0x15

enum class CST816Gesture : uint8_t {
    None        = 0x00,
    SwipeUp     = 0x01,
    SwipeDown   = 0x02,
    SwipeLeft   = 0x03,
    SwipeRight  = 0x04,
    SingleClick = 0x05,
    DoubleClick = 0x0B,
    LongPress   = 0x0C,
};

struct TouchPoint {
    int16_t x;
    int16_t y;
    bool    pressed;
    CST816Gesture gesture;
};

class CST816Touch {
public:
    CST816Touch(uint8_t sda, uint8_t scl, uint8_t irq, uint8_t rst);
    bool begin();
    bool read(TouchPoint &pt);
    bool available();

private:
    uint8_t _sda, _scl, _irq, _rst;
    void reset();
};

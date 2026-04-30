#pragma once
// On ESP32-C3 Arduino 2.x with ARDUINO_USB_CDC_ON_BOOT=1, HardwareSerial.h
// removes the 'Serial' extern but HWCDC.h in this version (2.0.14) does not
// re-declare it in a way the compiler sees.  Declare it explicitly.
#if ARDUINO_USB_CDC_ON_BOOT
#include "HWCDC.h"
extern HWCDC Serial;
#endif

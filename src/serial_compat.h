#pragma once
// On ESP32-C3 Arduino 2.x with ARDUINO_USB_CDC_ON_BOOT=1:
//   - HardwareSerial.h removes the 'Serial' HardwareSerial extern
//   - HWCDC.h re-declares 'Serial' as a HWCDC (USB CDC) instance
//   - But Arduino.h does not include HWCDC.h automatically
// Including HWCDC.h here restores 'Serial' for every file that needs it.
#if ARDUINO_USB_CDC_ON_BOOT
#include "HWCDC.h"
#endif
